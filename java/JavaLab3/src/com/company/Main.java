package com.company;

import ru.spbstu.amd.javaed.pipeline.worker.MutableWorker;


public class Main {

    public static void main(String[] args) {
        if(args.length == 0){
            String err = "Task config file have to be translated to the program";
            if(!Log.write(err)){
                System.out.println(err);
            }
            return;
        }
        MyPipelineManager manager = new MyPipelineManager();
        manager.setTaskConfig(args[0]).run();
    }
}
