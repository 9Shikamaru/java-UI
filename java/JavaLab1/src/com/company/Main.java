package com.company;

public class Main {

    public static void main(String[] args) {
        if(args.length == 0){
            String err = "Task config file have to be translated to the program";
            if(!Log.write(err)){
                System.out.println(err);
            }
            return;
        }
        Manager manager = new Manager();
        manager.runTask(args[0])
        .runTask(args[1]);
    }
}
