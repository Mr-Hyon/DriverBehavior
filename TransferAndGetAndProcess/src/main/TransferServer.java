package main;

import java.io.IOException;
import java.io.InputStream;
import java.net.ServerSocket;
import java.net.Socket;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.URL;
import java.net.URLConnection;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;

import com.google.gson.Gson;
import com.google.gson.JsonParser;

public class TransferServer{
    //  所有异常全部上抛
	

	
	public static void main(String srags[]) throws IOException {
		start();
	}
	
    public static void start() throws IOException{
        System.out.println("服务器启动中");
        //  创建服务器端ServerSocket类型的服务端Socket对象  端口号要和客户端一致
        ServerSocket sSocket = new ServerSocket(48081);
        //  通过服务器端socket对象获取对应连接进入的客户端socket对象
        System.out.println("服务器等待客户端连入...");
        //  具体连入的客户端(创建的socket)  accpet()方法
        while(true) {
        	Socket socket = sSocket.accept();  // 阻塞状态
            //  确定连入者
            String name = socket.getInetAddress().getHostAddress();
            //  通过客户端具体的客户端socket对象与其对应的客户端进行交互
            InputStream in = socket.getInputStream();
            //  注: 通过socket 通道中的字节流按照1024个字节(缓冲区大小) 持续不断的读取socket通道中的信息
            byte[] buf = new byte[1024];
            int len = 0;
            while((len = in.read(buf)) != -1){
                String data = new String(buf,0,len);
            	System.out.println(data);
                if(data.startsWith("{")&&data.endsWith("\n")) {
                	System.out.println("hello");
                	request(data.substring(0,data.length()-1));
                	System.out.print(data.substring(0,data.length()-1));
                }
                buf = new byte[1024];
            }
            // 关闭服务
            socket.close();
        }
        
    }
    public static void request(String content) {
    	String url = "http://localhost:48080/api/v1/event";
        String param = "";
        String sr = HttpRequest.sendPost(url, content);
        System.out.println(sr);
    }
    
}
    

    
   
