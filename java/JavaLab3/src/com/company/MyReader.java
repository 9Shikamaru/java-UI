package com.company;

import org.jetbrains.annotations.NotNull;
import ru.spbstu.amd.javaed.pipeline.io.MutableReader;

import java.io.*;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Optional;
import java.util.Set;

public class MyReader implements MutableReader {
    private static final Class<?>[] posibleOutputTypesArray = {String.class,byte[].class,char[].class};
    private static final HashSet<Class<?>> posibleOutputTypes = new HashSet<>(Arrays.asList(posibleOutputTypesArray));
    private int dataPortion;
    private BufferedInputStream inputStream;
    private boolean isEnd = false;

    MyReader(String inputFile, int dataPortion) throws ReaderException{
        this.dataPortion = dataPortion;
        try {
            inputStream = new BufferedInputStream(new FileInputStream(inputFile));
        } catch (FileNotFoundException e) {
            throw new ReaderException("MyReader: cannot open input file on reading");
        }
    }

    MyReader(String inputFile){
        this(inputFile, 14);
    }

    private void readDataPortion(byte[] readBuffer){
        try {
            if(inputStream.available() == 0){
                closeConnection();
                readBuffer = null;
                return;
            }

            if(inputStream.available() < dataPortion){
                readBuffer = new byte[inputStream.available()];
            }

            if(inputStream.read(readBuffer) == -1){
                closeConnection();
                return;
            }
        } catch (IOException e) {
             String errorMessage = "Error during reading input file";
            if(!Log.write(errorMessage)){
                System.out.println(errorMessage);
            }
            readBuffer = null;
            isEnd = true;
        }
    }

    private void closeConnection() {
        isEnd = true;
        try {
            inputStream.close();
        } catch (IOException e) {
            String errorMessage = "Error during reading input file";
            if(!Log.write(errorMessage)){
                System.out.println(errorMessage);
            }
        }
    }

    @Override
    public @NotNull Set<Class<?>> getPossibleOutputTypes() {
        return posibleOutputTypes;
    }

    @Override
    public @NotNull DataAccessor getDataAccessor(@NotNull Class<?> aClass) {
        return new MyDataDataAccessor(aClass);
    }


    public class MyDataDataAccessor implements DataAccessor{
        private Class<?> outputType;
        private byte[] readBuffer;

        MyDataDataAccessor(Class<?> outputType){
            this.outputType = outputType;
            readBuffer = new byte[dataPortion];
        }

        @Override
        public @NotNull Optional<Object> get() {
            readDataPortion(readBuffer);
            if(readBuffer==null){
                return Optional.empty();
            }else{
                if(outputType == byte[].class){
                    return Optional.ofNullable(readBuffer);
                }
                try {
                    if(outputType == char[].class){
                        return Optional.ofNullable(new String(readBuffer, "windows-1251").toCharArray());
                    }
                    if(outputType == String.class){
                        return Optional.ofNullable(new String(readBuffer, "windows-1251"));
                    }
                } catch (UnsupportedEncodingException e) {
                    String errorMessage = "MyReader: Error during encoding";
                    if(!Log.write(errorMessage)){
                        System.out.println(errorMessage);
                    }
                    closeConnection();
                    return Optional.empty();
                }
            }
            return Optional.empty();
        }

        @Override
        public boolean isEnd() {
            return MyReader.this.isEnd;
        }
    }

}
