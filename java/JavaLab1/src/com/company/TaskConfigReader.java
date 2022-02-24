package com.company;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.regex.Pattern;

enum TaskConfigParam{
    INPUTFILE,
    OUTPUTFILE,
    WORKER_CONFIG
}

public class TaskConfigReader {
    private HashMap<TaskConfigParam,String> taskConfig = null;
    private static final String rgexpMask = "^\\D+=\\S+.txt$";
    private static final String splitter = "=";

    public HashMap<TaskConfigParam,String> readConfig(String taskConfigFile) throws TaskConfigReaderException{
        taskConfig = new HashMap<>();
        try (BufferedReader br = new BufferedReader(new FileReader(taskConfigFile))) {
            String line;
            while ((line = br.readLine()) != null) {
                if(!(Pattern.matches(rgexpMask,line))){
                    throw new TaskConfigReaderException("Invalid syntax in "+taskConfigFile+". lines should be 'TaskConfigParam(String)=fileName(String)'");
                }
                String[] configSubLines = line.split(splitter);
                try {
                    taskConfig.put(TaskConfigParam.valueOf(configSubLines[0]),configSubLines[1]);
                }catch (IllegalArgumentException e){
                    throw new TaskConfigReaderException("Unknown param was translated in "+taskConfigFile+", file should contain INPUTFILE,OUTPUTFILE,WORKER_CONFIG");
                }
            }
            if(taskConfig.get(TaskConfigParam.INPUTFILE)==null || taskConfig.get(TaskConfigParam.WORKER_CONFIG)==null){
                throw new TaskConfigReaderException("INPUTFILE and WORKER_CONFIG params are obligated");
            }
            if(!Files.exists(Paths.get(taskConfig.get(TaskConfigParam.INPUTFILE))) || !Files.exists(Paths.get(taskConfig.get(TaskConfigParam.WORKER_CONFIG)))){
                throw new TaskConfigReaderException("INPUTFILE and WORKER_CONFIG have to exist");
            }
        } catch (IOException e) {
            throw new TaskConfigReaderException("Can not read " + taskConfigFile);
        }
        return taskConfig;
    }
}
