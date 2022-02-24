package com.company;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

import static java.nio.file.StandardOpenOption.*;

public class Log {
    private Log(){}
    private static final String logFile = "./logfile.txt";
    private static final Path logPath = Paths.get(logFile);

    static public boolean write(String logMessage){
        try {
            Files.write(logPath,(java.time.LocalDateTime.now().toString()+": ").getBytes(),CREATE,APPEND);
            Files.write(logPath,(logMessage+"\n").getBytes(),APPEND);
        } catch (IOException e) {
            return false;
        }
        return true;
    }
}
