package com.company;

import java.math.BigDecimal;
import java.math.RoundingMode;
import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;
import java.util.*;


public class ArithmeticCoder {

    private class Segment{
        double left;
        double right;
        Segment(double left,double right){
            this.left = left;
            this.right = right;
        }

        public void changeBorders(double left, double right){
            this.left = left;
            this.right = right;
        }
    }

    private HashMap<Byte, Double> probabilityTable = null;// Table of probabilities of symbols in text.
    private HashMap<Byte,Segment> segments = null;// Probability segments of symbols in text on [0,1] interval.
    // Maximum size of input byte array for which encode method can guarantee that decoding wll be accurate.
    // Can be set, if an input bytes are monotone.
    private int maxInputSize = 14;
    public int getMaxInputSize() {
        return maxInputSize;
    }
    public void setMaxInputSize(int maxInputSize) {
        this.maxInputSize = maxInputSize;
    }

    private void defineSegments(HashMap<Byte,Double> probabilityTable) throws ArithmeticCoderException {
        segments = new HashMap<>();
        double l = 0;
        for(Byte el : probabilityTable.keySet()){
            Double probability = probabilityTable.get(el);
            if(probability >1 || probability<0){
                throw new ArithmeticCoderException("Incorrect probability table was translated.Make sure probs are in [0;1] borders");
            }
            segments.put(el, new Segment(l,l = (l + probability)));
        }
    }

    // Returning value class. Object of this class is returned by encode.
    public class ArithmeticCodeOutput{
        private final double codeInDouble;
        private final String codeInString;
        private final HashMap<Byte, Double> usedProbabilityTable;
        private final int byteNumber;
        private final byte[] byteOutput; // final byte array output that contains, size of byteOutput info part, bytes of codeInDouble,
        // number of coded bytes, size of usedProbabilityTable and usedProbabilityTable

        ArithmeticCodeOutput(double codeInDouble, int byteNumber, HashMap<Byte, Double> usedProbabilityTable){
            this.codeInDouble = codeInDouble;
            this.byteNumber = byteNumber;
            this.usedProbabilityTable = usedProbabilityTable;
            this.codeInString = String.valueOf(codeInDouble);
            int sizeOfByteCode = Double.BYTES+Integer.BYTES+Integer.BYTES+(Double.BYTES+1)*this.usedProbabilityTable.size();
            byteOutput = new byte[Integer.BYTES + sizeOfByteCode];
            ByteBuffer buffer = ByteBuffer.wrap(byteOutput);
            buffer.putInt(sizeOfByteCode);
            buffer.putDouble(this.codeInDouble);
            buffer.putInt(this.byteNumber);
            buffer.putInt(this.usedProbabilityTable.size());
            for(byte el : this.usedProbabilityTable.keySet()){
                buffer.put(el);
                buffer.putDouble(this.usedProbabilityTable.get(el));
            }
        }
        public double getCodeInDouble() {
            return codeInDouble;
        }
        public String getCodeInString() {
            return codeInString;
        }
        public HashMap<Byte, Double> getUsedProbabilityTable() {
            return usedProbabilityTable;
        }
        public byte[] getByteOutput() {
            return byteOutput;
        }
    }

    //Counting probability table from translated byte array.
    private void countProbabilityTable(List<Byte> dataList) throws ArithmeticCoderException {
        probabilityTable = new HashMap<>();
        Set<Byte> dataSet = new HashSet<>(dataList);
        for(Byte elm : dataSet){
            double prob = (double) Collections.frequency(dataList,elm)/(dataList.size());
            probabilityTable.put(elm,prob);
        }
        defineSegments(probabilityTable);
    }

    public ArithmeticCodeOutput encode(byte[] data) throws ArithmeticCoderException{
        if(data.length > maxInputSize){
            throw new ArithmeticCoderException("Max input size was exceeded. " +
                    "ArithmeticCoder can't guarantee correctness of decoding input larger than max limit");
        }
        List<Byte> dataList = new ArrayList<>();
        for(byte el : data){
            dataList.add(el);
        }
        countProbabilityTable(dataList);
        Segment resultSegment = new Segment(0,1);
        dataList.forEach(elm ->{
            double right = resultSegment.left + (resultSegment.right - resultSegment.left)*segments.get(elm).right;
            double left = resultSegment.left + (resultSegment.right - resultSegment.left)*segments.get(elm).left;
            resultSegment.changeBorders(left,right);
        });
        return new ArithmeticCodeOutput((resultSegment.right + resultSegment.left)/2,data.length,probabilityTable);
    }

    public byte[] decode(byte[] data) throws ArithmeticCoderException{
        double code;
        int bytesNumber;
        int tableSize;
        probabilityTable = null;
        ByteBuffer buffer = ByteBuffer.wrap(data);
        // Parsing coded byte array and translating code to the main decode method, if this byte array wasn't generated by encode method throwing an error.
        try {
            code = buffer.getDouble();
            bytesNumber = buffer.getInt();
            tableSize = buffer.getInt();
            probabilityTable = new HashMap<>();
            for(int i=0;i<tableSize;i++){
                byte elm = buffer.get();
                double probability = buffer.getDouble();
                probabilityTable.put(elm,probability);
            }
        }catch (BufferUnderflowException e){
            throw new ArithmeticCoderException("Translated to the decode method byte array is incorrect. " +
                    "Make sure that it was produced by ArithmeticCoder with encode");
        }
        return decode(code,bytesNumber,probabilityTable);
    }

    public byte[] decode(double code, int bytesNumber, HashMap<Byte, Double> probabilityTable) throws ArithmeticCoderException{
        defineSegments(probabilityTable);
        if(code>1 || code<0){
            throw new ArithmeticCoderException("Incorrect code was translated to the method.");
        }
        byte[] result = new byte[bytesNumber];
        for(int i=0;i<bytesNumber;i++){
            for(byte elm : segments.keySet()){
                Segment elmSegment = segments.get(elm);
                if(code >= elmSegment.left && code < elmSegment.right){
                    result[i]=elm;
                    code = (code - elmSegment.left)/(elmSegment.right - elmSegment.left);
                    break;
                }
            }
        }
        return result;
    }
}