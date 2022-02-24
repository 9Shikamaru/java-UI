package com.company;

import java.io.*;
import java.nio.ByteBuffer;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import static com.company.WorkModes.*;
import static java.nio.file.StandardOpenOption.*;

enum WorkerConfigParam{
    WORKMODE
}

enum WorkModes{
    encode,
    decode
}

public class Worker {

    private String inputFile=null;
    private String outputFile=null;
    private static final String defaultOutputFile = "./outputfile.txt";
    private static final String readableOutputFileEnding = "_readable.txt";// Ending that needed for forming readable output file for encoding
    private static final String regexpMaskGeneral = "^\\D+=\\S+$";
    private static final String separator = "=";
    private static final String regexpMaskTxtFile = "^\\S+\\.txt$";
    private static final String regexpMaskTxtFileEnding = "\\.txt$";
    HashMap<WorkerConfigParam,String> workerConfig = null;

    Worker(){}
    Worker(String workerConfigFile) throws WorkerException{
        setWorkConfig(workerConfigFile);
    }

    // Reading and parsing worker config file
    public Worker setWorkConfig(String workerConfigFile) throws WorkerException{
        workerConfig = new HashMap<>();
        try (BufferedReader br = new BufferedReader(new FileReader(workerConfigFile))) {
            String line;
            while ((line = br.readLine()) != null) {
                if(!(Pattern.matches(regexpMaskGeneral,line))){
                    throw new WorkerException("Invalid syntax in ." + workerConfigFile + " lines should be 'WorkerConfigParam(String)=value(String)'");
                }
                String[] configSubLines = line.split(separator);
                workerConfig.put(WorkerConfigParam.valueOf(configSubLines[0]),configSubLines[1]);
            }
        } catch (IOException e) {
            throw new WorkerException("Can not read " + workerConfigFile);
        }
        if(workerConfig.get(WorkerConfigParam.WORKMODE)==null){
            throw new WorkerException("WORKMODE param is obligated");
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
                        throw new WorkerException("WORKMODE have to be 'encode' or 'decode'");
                    }
                    break;
            }
        }
        return this;
    }

    // If output file wasn't translated.
    public Worker work(String inputFile) throws WorkerException{
        work(inputFile,defaultOutputFile);
        return this;
    }

    public Worker work(String inputFile,String outputFile) throws WorkerException{
        if(!Pattern.matches(regexpMaskTxtFile,inputFile) || !(Pattern.matches(regexpMaskTxtFile,outputFile))){
            throw new WorkerException("Input and output files have to be txt files");
        }
        this.inputFile = inputFile;
        this.outputFile = outputFile;
        if(workerConfig == null){
            throw new WorkerException("worker config have to be set");
        }
        // Clearing output file.
        if(Files.exists(Paths.get(this.outputFile))){
            try {
                Files.delete(Paths.get(this.outputFile));
            } catch (IOException e) {
                throw new WorkerException("Cant delete " + this.outputFile + " from previous usages");
            }
        }
        try {
            ArithmeticCoder coder = new ArithmeticCoder();
            try (BufferedInputStream inputStream = new BufferedInputStream(new FileInputStream(this.inputFile))){
                if(inputStream.available() == 0){
                    throw new WorkerException("An empty file was translated to the worker in " + this.inputFile);
                }
                switch (WorkModes.valueOf(workerConfig.get(WorkerConfigParam.WORKMODE))){
                    case encode:
                        byte[] bufferEncode;
                        ArrayList<String> resultsStringList = new ArrayList<>();
                        if(inputStream.available() < coder.getMaxInputSize()){
                            bufferEncode = new byte[inputStream.available()];
                        }else{
                            bufferEncode = new byte[coder.getMaxInputSize()];
                        }
                        while (inputStream.read(bufferEncode) != -1) {
                            ArithmeticCoder.ArithmeticCodeOutput result = coder.encode(bufferEncode);
                            resultsStringList.add(result.getCodeInString());
                            try {
                                Files.write(Paths.get(this.outputFile), result.getByteOutput(),CREATE,APPEND);
                            } catch (IOException e) {
                                throw new WorkerException("Can not write to the " + this.outputFile);
                            }
                            if(inputStream.available() < coder.getMaxInputSize() && inputStream.available()!=0){
                                bufferEncode = new byte[inputStream.available()];
                            }
                        }
                        // Saving double codes in readable format in specially created readable output file.
                        try {
                            Matcher fileEndPosition = Pattern.compile(regexpMaskTxtFileEnding).matcher(this.outputFile);
                            fileEndPosition.find();
                            String readableOutputFile = this.outputFile.substring(0, fileEndPosition.start()) + readableOutputFileEnding;
                            if(Files.exists(Paths.get(readableOutputFile))){
                                Files.delete(Paths.get(readableOutputFile));
                            }
                            Files.write(Paths.get(readableOutputFile),resultsStringList,CREATE);
                        } catch (IOException e) {
                            throw new WorkerException("Can not write in readableOutputFile file");
                        }
                        break;
                    case decode:
                        byte[] sizeNextBlockBuffer = new byte[Integer.BYTES];
                        while (inputStream.read(sizeNextBlockBuffer) != -1) {
                            byte[] bufferDecode = new byte[ByteBuffer.wrap(sizeNextBlockBuffer).getInt()];
                            if(inputStream.read(bufferDecode) != -1){
                                byte[] result = coder.decode(bufferDecode);
                                try {
                                    Files.write(Paths.get(this.outputFile), result,CREATE,APPEND);
                                } catch (IOException e) {
                                    throw new WorkerException("Can not write to the " + this.outputFile);
                                }
                            }else {
                                throw new WorkerException("Something wrong with input data for decoding process. " +
                                        "Make sure that it was generated by ArithmeticCoder and saved by Worker");
                            }
                        }
                        break;
                }
            }catch (IOException e) {
                throw new WorkerException("Can not read " + this.inputFile);
            }
        } catch (ArithmeticCoderException e) {
            throw new WorkerException("ArithmeticCoder: " + e.getMessage());
        }
        return this;
    }
}
