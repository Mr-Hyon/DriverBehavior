package main;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Timer;
import java.util.TimerTask;

import com.google.gson.Gson;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonParser;

public class DataProcessor{
	
	int front_count=0;
	int left_count=0;
	int right_count=0;
    int count=0;
    	ArrayList<Double> timeTag_list=new ArrayList<Double>();
    	
    	public static void main(String args[]) {
    		DataProcessor processor=new DataProcessor();
    		processor.receive("{\"id\":\"5d1c99400e360800019ecc0d\",\"pushed\":0,\"device\":\"device\",\"created\":1562155328963,\"modified\":0,\"origin\":0,\"schedule\":null,\"event\":null,\"readings\":[{\"id\":\"5d1c99400e360800019ecc0e\",\"pushed\":0,\"created\":1562155328963,\"origin\":0,\"modified\":0,\"device\":\"device\",\"name\":\"ax\",\"value\":\"-0.00\"}");
    		processor.print_all();
    	}
    	
    	public void receive(String str_json) {
    		Gson gson=new Gson();
    		ArrayList<Double> list=transfer(str_json);
    		process2(list);
    	}
    	
    	public  ArrayList<Double> transfer(String str) {
    		ArrayList<Double> list=new ArrayList<Double>();
    		JsonParser parser=new JsonParser();
    		JsonElement element= parser.parse(str);
    		JsonArray array=element.getAsJsonObject().get("readings").getAsJsonArray();
    		Iterator<JsonElement> iter=array.iterator();
    		while(iter.hasNext()) {
    			double num=iter.next().getAsJsonObject().get("value").getAsDouble();
    			list.add(num);
    		}
    		return list;
    	}
    	
    	public void process2(ArrayList<Double> list) {
    		Double ax=list.get(0);
    		Double ay=list.get(1);
    		Double az=list.get(2);
    		Double gx=list.get(3);
    		Double gy=list.get(4);
    		Double gz=list.get(5);
    		count+=1;
    		if(ax>=1.3&&ax<=2.5&&gx>=(-0.12)&&gx<=(-0.08)){
    			System.out.println("Safe acceleration");
    		}
    		else if(ax>=(-2.5)&&ax<=(-1.3)&&gx>=0.08&&gx<=0.12){
    			System.out.println("Safe deceleration");
    		}
    		else if(ax>2.5&&gx<(-0.12)){
    			front_count+=1;
    			System.out.println("Hard acceleration");
    		}
    		else if(ax<(-2.5)&&gx>0.12){
    			front_count+=1;
    			System.out.println("Hard deceleration");
    		}
    		else {
    			System.out.println("At a stable speed");
    		}
    		if(ay>=(-3.0)&&ay<=(-1.8)&&gy>=0.1&&gy<=0.3){
    			System.out.println("Safe left turn");
    		}
    		else if(ay>=1.8&&ay<=3.0&&gy>=(-0.3)&&gy<=(-0.1)){
    			System.out.println("Safe right turn");
    		}
    		else if(ay<(-3.0)&&gy>0.3){
    			left_count+=1;
    			System.out.println("Sharp left turn");
    		}
    		else if(ay>3.0&&gy<(-0.3)){
    			right_count+=1;
    			System.out.println("Sharp right turn");
    		}
    		else {
    			System.out.println("Going straight");
    		}
    	}
    	
    	public void print_all() {
    		Timer timer=new Timer();
    		timer.schedule(new TimerTask() {

				@Override
				public void run() {
					print_all();
					System.out.println("front:"+front_count);
					System.out.println("left:"+left_count);
					System.out.println("right:"+right_count);
					System.out.println("count:"+count);
				}
    		}, 8000);
    	}
    	
    	
    }