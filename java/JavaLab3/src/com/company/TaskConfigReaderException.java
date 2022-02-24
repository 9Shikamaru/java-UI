package com.company;

public class TaskConfigReaderException extends Exception {
    TaskConfigReaderException(String readerName, String message){
        super(readerName+ ": " + message);
    }
}
