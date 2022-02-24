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
    DATAPORTION
}

public class TaskConfigReader {

    public class TaskConfig{
        public HashMap<TaskConfigParam,String> taskParamsConfig = new HashMap<>();
        public HashMap<Integer,String> taskWorkersConfig = new HashMap<>();
        public final String workerSplitter = ",";
    }

    private TaskConfig taskConfig = new TaskConfig();
    private static final String rgexpGlobal = "^\\S+=\\S+$";
    private static final String rgexpFileMask = "^\\D+=\\S+.txt$";
    private static final String rgexpWorkerMask = "^WORKER\\d+$";
    private static final String rgexpWorkerConfMask = "^[^\\s,]+,[^\\s,]+.txt$";
    private static final String rgexpNumber = "\\D+";
    private static final String splitter = "=";

    public TaskConfig readConfig(String taskConfigFile) throws TaskConfigReaderException{

        try (BufferedReader br = new BufferedReader(new FileReader(taskConfigFile))) {
            String line;
            while ((line = br.readLine()) != null) {
                if(!(Pattern.matches(rgexpGlobal,line))){
                    throw new TaskConfigReaderException(this.toString(),"Invalid syntax in task config file.");
                }
                String[] configSubLines = line.split(splitter);
                if(Pattern.matches(rgexpWorkerMask,configSubLines[0])){
                    String workerNumber = configSubLines[0].replaceAll(rgexpNumber,"");
                    if(!Pattern.matches(rgexpWorkerConfMask,configSubLines[1])){
                        throw new TaskConfigReaderException(this.toString(),"Invalid syntax in worker config value. value have to be 'workerClassName,workerConfigFile'");
                    }
                    taskConfig.taskWorkersConfig.put(Integer.parseInt(workerNumber),configSubLines[1]);
                }else{
                    try {
                        if(TaskConfigParam.valueOf(configSubLines[0]) == TaskConfigParam.DATAPORTION){
                            try {
                                Integer.parseInt(configSubLines[1]);
                            }catch (NumberFormatException e){
                                throw new TaskConfigReaderException(this.toString(),"Invalid syntax in task config data portion value. value have to be integer");
                            }
                        }
                        taskConfig.taskParamsConfig.put(TaskConfigParam.valueOf(configSubLines[0]),configSubLines[1]);
                    }catch (IllegalArgumentException e){
                        throw new TaskConfigReaderException(this.toString(),"Unknown param was translated in task config file, file should contain INPUTFILE,OUTPUTFILE,WORKER_CONFIG");
                    }
                }
            }
            if(taskConfig.taskParamsConfig.get(TaskConfigParam.INPUTFILE)==null /* || taskConfig.taskParamsConfig.get(TaskConfigParam.DATAPORTION)==null */){
                throw new TaskConfigReaderException(this.toString(),"INPUTFILE and DATAPORTION params are obligated");
            }
            if(!Files.exists(Paths.get(taskConfig.taskParamsConfig.get(TaskConfigParam.INPUTFILE)))){
                throw new TaskConfigReaderException(this.toString(),"INPUTFILE have to exist");
            }
            if(taskConfig.taskWorkersConfig.size()==0){
                throw new TaskConfigReaderException(this.toString(),"At least one Worker have to be set");
            }
        } catch (IOException e) {
            throw new TaskConfigReaderException(this.toString(),"Can not read task config file");
        }
        return taskConfig;
    }
}
