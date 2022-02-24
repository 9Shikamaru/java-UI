package com.company;

import org.jetbrains.annotations.NotNull;
import ru.spbstu.amd.javaed.pipeline.worker.*;

import java.io.*;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.*;
import java.util.regex.Pattern;

import static java.nio.file.StandardOpenOption.APPEND;
import static java.nio.file.StandardOpenOption.CREATE;

enum WorkerConfigParam{
    WORKMODE
}

enum WorkModes{
    encode,
    decode
}

public class MyWorker implements MutableWorker {

    private static final String regexpMaskGeneral = "^\\D+=\\S+$";
    private static final String separator = "=";
    HashMap<WorkerConfigParam,String> workerConfig = null;
    private static final Class<?>[] posibleInputTypesArray = {String.class,byte[].class,char[].class};
    private static final HashSet<Class<?>> posibleInputTypes = new HashSet<>(Arrays.asList(posibleInputTypesArray));
    private PossibleOutputType outputType = null;
    private Worker nextWorker = null;

   public MyWorker(){}
   private MyWorker(String workerConfigFile) throws WorkerException{
        setWorkConfig(workerConfigFile);
    }
    public static MyWorker of(String workerConfigFile){
        return new MyWorker(workerConfigFile);
    }

    // Reading and parsing worker config file
    public MyWorker setWorkConfig(String workerConfigFile) throws WorkerException{
        workerConfig = new HashMap<>();
        try (BufferedReader br = new BufferedReader(new FileReader(workerConfigFile))) {
            String line;
            while ((line = br.readLine()) != null) {
                if(!(Pattern.matches(regexpMaskGeneral,line))){
                    throw new WorkerException(this.toString(),"Invalid syntax in ." + workerConfigFile + " lines should be 'WorkerConfigParam(String)=value(String)'");
                }
                String[] configSubLines = line.split(separator);
                workerConfig.put(WorkerConfigParam.valueOf(configSubLines[0]),configSubLines[1]);
            }
        } catch (IOException e) {
            throw new WorkerException(this.toString(),"Can not read " + workerConfigFile);
        }
        if(workerConfig.get(WorkerConfigParam.WORKMODE)==null){
            throw new WorkerException(this.toString(),"WORKMODE param is obligated");
        }
        // Checking correctness of workerConfig. Loop is needed for expansion WorkerConfigParam
        for(WorkerConfigParam param : workerConfig.keySet()){
            switch (param){
                case WORKMODE:
                    boolean correctMode = false;
                    for(WorkModes mode: WorkModes.values()){
                        if(workerConfig.get(param).equalsIgnoreCase(mode.toString())){
                            correctMode = true;
                            break;
                        }
                    }
                    if(!correctMode){
                        throw new WorkerException(this.toString(),"WORKMODE have to be 'encode' or 'decode'");
                    }
                    break;
            }
        }
        return this;
    }

    @Override
    public void setNext(@NotNull Worker worker) {
        outer:
        for(Class<?> inputType : worker.getPossibleInputTypes()){
            for(PossibleOutputType posibleType : PossibleOutputType.values()){
                if(inputType == posibleType.getType()){
                    outputType = posibleType;
                    nextWorker = worker;
                    break outer;
                }
            }
        }
        if (outputType == null){
            throw new NoCommonTypesException(this.toString(),nextWorker.toString(),"No common type, make sure that"+nextWorker.toString()+
                    "support any of String, byte[] or char[] type");
        }
    }

    @Override
    public @NotNull Set<Class<?>> getPossibleInputTypes() {
        return posibleInputTypes;
    }

    @Override
    public long work(@NotNull Object data, @NotNull Object prev) {
        if(workerConfig == null){
            throw new WorkException(this.toString(),"worker config have to be set");
        }
        byte[] workingData;
        if(data instanceof byte[]){
            workingData = (byte[])data;
        }else if(data instanceof char[]){
            try {
                workingData = new String((char[])data).getBytes("windows-1251");
            } catch (UnsupportedEncodingException e) {
                throw new WorkException(this.toString(),"UnsupportedEncoding data was translated to worker");
            }
        }else if(data instanceof String){
            try {
                workingData = ((String)data).getBytes("windows-1251");
            } catch (UnsupportedEncodingException e) {
                throw new WorkException(this.toString(),"UnsupportedEncoding data was translated to worker");
            }
        }else {
            throw new WorkException(this.toString(),"Data of unsupported type was translated to the work method");
        }
        try {
            ArithmeticCoder coder = new ArithmeticCoder();
            byte[] result;
            ArrayList<byte[]> resultList = new ArrayList<>();
            switch (WorkModes.valueOf(workerConfig.get(WorkerConfigParam.WORKMODE))){
                case encode:
                    byte[] encodeBuffer = new byte[coder.getMaxInputSize()];
                    int posInEncodeBuffer = 0;
                    for(byte elm : workingData){
                        if(posInEncodeBuffer == coder.getMaxInputSize()){
                            resultList.add(coder.encode(encodeBuffer).getByteOutput());
                            encodeBuffer[0] = elm;
                            posInEncodeBuffer = 1;
                        }else {
                            encodeBuffer[posInEncodeBuffer] = elm;
                            posInEncodeBuffer++;
                        }
                    }
                    encodeBuffer = Arrays.copyOf(encodeBuffer,posInEncodeBuffer);
                    resultList.add(coder.encode(encodeBuffer).getByteOutput());
                    break;
                case decode:
                    byte[] sizeNextBlockBuffer = new byte[Integer.BYTES];
                    int sizeNextBlock;
                    int iter = 0;
                    boolean readSizeNextBlockBuffer = true;
                    while(iter<workingData.length){
                        if(readSizeNextBlockBuffer){
                            if(workingData.length - iter < sizeNextBlockBuffer.length){
                                throw new WorkException(this.toString(),"Incorrect input was translated to the decode method. Problems in block size reading.");
                            }
                            for(int j=0;j<sizeNextBlockBuffer.length;j++){
                                sizeNextBlockBuffer[j] = workingData[iter++];
                            }
                            readSizeNextBlockBuffer = false;
                        }else{
                            sizeNextBlock = ByteBuffer.wrap(sizeNextBlockBuffer).getInt();
                            byte[] decodeBuffer = new byte[sizeNextBlock];
                            if(workingData.length - iter < decodeBuffer.length){
                                throw new WorkException(this.toString(),"Incorrect input was translated to the decode method. Problems in code block reading.");
                            }
                            for(int j=0;j<decodeBuffer.length;j++){
                                decodeBuffer[j] = workingData[iter++];
                            }
                            resultList.add(coder.decode(decodeBuffer));
                            if(sizeNextBlock % 2 != 0){
                                iter++;
                            }
                            readSizeNextBlockBuffer = true;
                        }
                    }
                    break;
            }
            int finalOutputSize = 0;
            for(byte[] arr : resultList){
                finalOutputSize+=arr.length;
            }
            result = new byte[finalOutputSize];
            int i = 0;
            for(byte[] arr : resultList){
                for(byte elm : arr){
                    result[i++] = elm;
                }
            }
            try {
                switch (outputType){
                    case STRING:
                        nextWorker.work(new String(result, "windows-1251"),this);
                        break;
                    case BYTE_ARRAY:
                        nextWorker.work(result,this);
                        break;
                    case CHAR_ARRAY:
                        nextWorker.work(new String(result, "windows-1251").toCharArray(),this);
                        break;
                }
            }catch (UnsupportedEncodingException e) {
            e.printStackTrace();
            }
        } catch (ArithmeticCoderException e) {
            throw new WorkException(this.toString(),"ArithmeticCoder: " + e.getMessage());
        }
        return 0;
    }
}
