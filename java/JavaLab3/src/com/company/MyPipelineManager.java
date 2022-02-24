package com.company;

import ru.spbstu.amd.javaed.pipeline.Pipeline;
import ru.spbstu.amd.javaed.pipeline.PipelineManager;
import ru.spbstu.amd.javaed.pipeline.worker.*;

import java.io.*;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;

public class MyPipelineManager implements PipelineManager {
    private ArrayList<MutableWorker> workers = new ArrayList<>();
    private TaskConfigReader taskConfigReader = new TaskConfigReader();
    private TaskConfigReader.TaskConfig taskConfig;
    private MyPipeline pipeline;
    private String errorMessage;
    private boolean taskSetError = true;

    public MyPipelineManager setTaskConfig(String taskConfigFile){
        taskSetError = false;
        try {
             taskConfig = taskConfigReader.readConfig(taskConfigFile);
        } catch (TaskConfigReaderException e) {
            if(!Log.write(e.getMessage())){
                System.out.println(e.getMessage());
            }
            taskSetError = true;
            return this;
        }

        pipelineCreation();
        return this;
    }

    private void pipelineCreation(){
        try {
            MutableWorker prevWorker = null;
            MutableWorker startingWorker = null;
            for(int i=1;i<=taskConfig.taskWorkersConfig.size();i++){
                String workerValueString = taskConfig.taskWorkersConfig.get(i);
                if(workerValueString == null){
                    errorMessage = "Missed worker in the workers chain. Make sure that your workers chain starts with WORKER1 and no one worker is missed.";
                    if(!Log.write(errorMessage)){
                        System.out.println(errorMessage);
                    }
                    taskSetError = true;
                }
                String[] workerClassNameAndConfig = workerValueString.split(taskConfig.workerSplitter);
                MutableWorker worker = ru.spbstu.amd.javaed.pipeline.worker.MutableWorker.of(
                        workerClassNameAndConfig[0], workerClassNameAndConfig[1]
                );
                if(i==1){
                    startingWorker = worker;
                } else{
                    prevWorker.setNext(worker);
                }
                if(i == taskConfig.taskWorkersConfig.size()){
                    String outputFile = taskConfig.taskParamsConfig.get(TaskConfigParam.OUTPUTFILE);
                    if(Files.exists(Paths.get(outputFile))){
                        try {
                            Files.delete(Paths.get(outputFile));
                        } catch (IOException e) {
                            if(!Log.write(e.getMessage())){
                                System.out.println(e.getMessage());
                            }
                            taskSetError = true;
                        }
                    }
                    MyWriter writer = new MyWriter(outputFile);
                    worker.setNext(writer);
                }
                prevWorker = worker;
                workers.add(worker);
            }
            MyReader reader;
            try {
                if(taskConfig.taskParamsConfig.get(TaskConfigParam.DATAPORTION)==null){
                    reader = new MyReader(taskConfig.taskParamsConfig.get(TaskConfigParam.INPUTFILE));
                }else{
                    int dataPortion = Integer.parseInt(taskConfig.taskParamsConfig.get(TaskConfigParam.DATAPORTION));
                    reader = new MyReader(taskConfig.taskParamsConfig.get(TaskConfigParam.INPUTFILE),
                            dataPortion);
                }
            }catch (ReaderException e){
                if(!Log.write(e.getMessage())){
                    System.out.println(e.getMessage());
                }
                taskSetError=true;
                return;
            }
            pipeline = new MyPipeline(reader, startingWorker);
        } catch (WorkerException e) {
            if(!Log.write(e.getWorkerName() + ": " + e.getMessage())){
                System.out.println(e.getMessage());
            }
        }
    }

    @Override
    public void run() {
        if(taskSetError){
            return;
        }
        pipeline.run();
    }

}
