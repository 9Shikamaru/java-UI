package com.company;

import org.jetbrains.annotations.NotNull;
import ru.spbstu.amd.javaed.pipeline.io.WriteException;
import ru.spbstu.amd.javaed.pipeline.io.Writer;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

import static java.nio.file.StandardOpenOption.APPEND;
import static java.nio.file.StandardOpenOption.CREATE;

public class MyWriter implements Writer {
    private static final Class<?>[] posibleInputTypes = {String.class,byte[].class,char[].class};
    private static final HashSet<Class<?>> setOfPosibleInputTypes = new HashSet<Class<?>>(Arrays.asList(posibleInputTypes));
    private String outputFile=null;
    private static final String defaultOutputFile = "./outputfile.txt";

    MyWriter(){
        this.outputFile = defaultOutputFile;
    }

    MyWriter(String outputFile){
        this.outputFile = outputFile;
    }

    @Override
    public long work(@NotNull Object data, @NotNull Object o) {
        try {
            if(data instanceof byte[]){
                Files.write(Paths.get(outputFile),(byte[])data,CREATE,APPEND);
            }else if(data instanceof char[]){
                Files.write(Paths.get(outputFile),new String((char[])data).getBytes("windows-1251"),CREATE,APPEND);
            }else if(data instanceof String){
                Files.write(Paths.get(outputFile),((String)data).getBytes("windows-1251"),CREATE,APPEND);
            }else {
                throw new WriteException(this.toString(),"Data of unsupported type was passed to the Writer");
            }
        } catch (IOException e) {
            throw new WriteException(this.toString(),"Can not write in outputFile file");
        }
        return 0;
    }

    @Override
    public @NotNull Set<Class<?>> getPossibleInputTypes() {
        return setOfPosibleInputTypes;
    }
}
