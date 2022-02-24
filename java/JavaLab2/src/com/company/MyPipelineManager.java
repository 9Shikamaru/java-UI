package com.company;

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
    private MutableWorker startingWorker = null;
    private PossibleOutputType readDataType = null;
    private int defaultDataPortion = 14;
    private String errorMessage;
    private boolean taskSetError;

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

        try {
            MutableWorker prevWorker = null;
            for(int i=1;i<=taskConfig.taskWorkersConfig.size();i++){
                String workerValueString = taskConfig.taskWorkersConfig.get(i);
                if(workerValueString == null){
                    errorMessage = "Missed worker in the workers chain. Make sure that your workers chain starts with WORKER1 and no one worker is missed.";
                    if(!Log.write(errorMessage)){
                        System.out.println(errorMessage);
                    }
                    taskSetError = true;
                    return this;
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
                            return this;
                        }
                    }
                    MyWriter writer = new MyWriter(outputFile);
                    worker.setNext(writer);
                }
                prevWorker = worker;
                workers.add(worker);
            }
        } catch (WorkerException e) {
            if(!Log.write(e.getWorkerName() + ": " + e.getMessage())){
                System.out.println(e.getMessage());
            }
        }

        readDataType = null;
        outer:
        for(Class<?> inputType : startingWorker.getPossibleInputTypes()){
            for(PossibleOutputType posibleType : PossibleOutputType.values()){
                if(inputType == posibleType.getType()){
                    readDataType = posibleType;
                    break outer;
                }
            }
        }
        if(readDataType == null){
            errorMessage = "No common type, make sure that"+startingWorker.toString()+
                    "support any of String, byte[] or char[] type";
            if(!Log.write(errorMessage)){
                System.out.println(errorMessage);
            }
            taskSetError = true;
        }
        return this;
    }

    @Override
    public void run() {
        if(taskSetError){
            return;
        }
        int dataPortion;
        if(taskConfig.taskParamsConfig.get(TaskConfigParam.DATAPORTION)==null){
            dataPortion = defaultDataPortion;
        }else{
            dataPortion = Integer.parseInt(taskConfig.taskParamsConfig.get(TaskConfigParam.DATAPORTION));
        }
        try (BufferedInputStream inputStream = new BufferedInputStream(new FileInputStream(taskConfig.taskParamsConfig.get(TaskConfigParam.INPUTFILE)))){
            byte[] readBuffer;
            if(inputStream.available() < dataPortion){
                readBuffer = new byte[inputStream.available()];
            }else{
                readBuffer = new byte[dataPortion];
            }
            while (inputStream.read(readBuffer) != -1){
                switch (readDataType){
                    case BYTE_ARRAY:
                        startingWorker.work(readBuffer,this);
                        break;
                    case CHAR_ARRAY:
                        startingWorker.work(new String(readBuffer, "windows-1251").toCharArray(),this);
                        break;
                    case STRING:
                        startingWorker.work(new String(readBuffer,"windows-1251"),this);
                        break;
                }
                if(inputStream.available() < dataPortion && inputStream.available()!=0){
                    readBuffer = new byte[inputStream.available()];
                }
            }
        }catch (IOException e) {
            errorMessage = "Can not read " + taskConfig.taskParamsConfig.get(TaskConfigParam.INPUTFILE);
            if(!Log.write(errorMessage)){
                System.out.println(errorMessage);
            }
        }catch (WorkerException e) {
            if(!Log.write(e.getWorkerName() + ": " + e.getMessage())){
                System.out.println(e.getMessage());
            }
        }
    }
}
