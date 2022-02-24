package com.company;

import java.util.HashMap;

public class Manager {
    private HashMap<TaskConfigParam,String> taskConfig = null;
    private Worker worker = new Worker();
    private TaskConfigReader taskConfigReader =  new TaskConfigReader();

    public Manager runTask(String taskConfigFile){
        try {
            taskConfig = taskConfigReader.readConfig(taskConfigFile);
        } catch (TaskConfigReaderException e) {
            if(!Log.write(e.getMessage())){
                System.out.println(e.getMessage());
            }
            return this;
        }

        try {
            worker.setWorkConfig(taskConfig.get(TaskConfigParam.WORKER_CONFIG));
            if(taskConfig.get(TaskConfigParam.OUTPUTFILE) == null){
                worker.work(taskConfig.get(TaskConfigParam.INPUTFILE));
            }else {
                worker.work(taskConfig.get(TaskConfigParam.INPUTFILE),taskConfig.get(TaskConfigParam.OUTPUTFILE));
            }
        } catch (WorkerException e) {
            if(!Log.write(e.getMessage())){
                System.out.println(e.getMessage());
            }
            return this;
        }

        return this;
    }
}
