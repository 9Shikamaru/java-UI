package com.company;

import ru.spbstu.amd.javaed.pipeline.Pipeline;
import ru.spbstu.amd.javaed.pipeline.io.Producer;
import ru.spbstu.amd.javaed.pipeline.worker.MutableWorker;
import ru.spbstu.amd.javaed.pipeline.worker.WorkerException;

import java.util.Optional;

public class MyPipeline implements Pipeline {
    private Producer.DataAccessor dataAccessor = null;
    private MutableWorker firstWorker = null;
    private boolean dataAccessorError = false;
    private String errorMessage;

    MyPipeline(MyReader reader, MutableWorker firstWorker){
        this.firstWorker = firstWorker;
        if(firstWorker == null){
            errorMessage = "No first worker was translated to the" + this.toString();
            if(!Log.write(errorMessage)){
                System.out.println(errorMessage);
            }
            dataAccessorError = true;
        }else {
            Class<?> chosenType = null;
            outer:
            for(Class<?> outputType : reader.getPossibleOutputTypes()){
                for(Class<?> inputType : firstWorker.getPossibleInputTypes()){
                    if(inputType == outputType){
                        chosenType = outputType;
                        break outer;
                    }
                }
            }

            if(chosenType == null){
                errorMessage = "No common type, make sure that"+firstWorker.toString()+"or"+reader.toString()+
                        "support any of String, byte[] or char[] type";
                if(!Log.write(errorMessage)){
                    System.out.println(errorMessage);
                }
                dataAccessorError = true;
            }else{
                this.dataAccessor = reader.getDataAccessor(chosenType);
            }
        }
    }

    @Override
    public void run() {
        if(dataAccessorError || (this.dataAccessor == null)){
            errorMessage = "Data Accessor wasn't initialized";
            if(!Log.write(errorMessage)){
                System.out.println(errorMessage);
            }
            return;
        }
        do{
            Optional<Object> data = dataAccessor.get();
            if (data.isPresent()) {
                try {
                    firstWorker.work(data.get(), this);
                }catch (WorkerException e) {
                    if(!Log.write(e.getWorkerName() + ": " + e.getMessage())){
                        System.out.println(e.getMessage());
                    }
                }
            }else{
                if(!dataAccessor.isEnd()){
                    errorMessage = "Pipeline: null was received from data accessor without end of file";
                    if(!Log.write(errorMessage)){
                        System.out.println(errorMessage);
                    }
                }
                return;
            }
        }while (!dataAccessor.isEnd());
    }
}
