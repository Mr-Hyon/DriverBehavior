package main;

import org.eclipse.paho.client.mqttv3.*;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;

import java.io.UnsupportedEncodingException;
import java.net.URI;
import java.net.URISyntaxException;

/**
 * A sample application that demonstrates how to use the Paho MQTT v3.1 Client blocking API.
 */
public class Subscriber implements MqttCallback {

    private final int qos = 1;
    private String topic = "EdgeXDataTopic";
    private String host="tcp://m24.cloudmqtt.com:14014";
    private String username="wbwzpwvj";
    private String password="N8cBbXsiIrek";
    private String clientId="MQTT-Java-Example";
    DataProcessor processor=new DataProcessor();
    
    private MqttClient client;

    public Subscriber() throws MqttException {
        MqttConnectOptions conOpt = new MqttConnectOptions();
        conOpt.setCleanSession(true);
        conOpt.setUserName(username);
        conOpt.setPassword(password.toCharArray());

        this.client = new MqttClient(host, clientId, new MemoryPersistence());
        this.client.setCallback(this);
        this.client.connect(conOpt);
        
        this.client.subscribe(this.topic, qos);
    }

    public void sendMessage(String payload) throws MqttException {
        MqttMessage message = new MqttMessage(payload.getBytes());
        message.setQos(qos);
        this.client.publish(this.topic, message); // Blocking publish
    }

    /**
     * @see MqttCallback#connectionLost(Throwable)
     */
    public void connectionLost(Throwable cause) {
        System.out.println("Connection lost because: " + cause);
        System.exit(1);
    }

    /**
     * @see MqttCallback#deliveryComplete(IMqttDeliveryToken)
     */
    public void deliveryComplete(IMqttDeliveryToken token) {
    	System.out.println("hello");
    }

    /**
     * @throws UnsupportedEncodingException 
     * @see MqttCallback#messageArrived(String, MqttMessage)
     */
    public void messageArrived(String topic, MqttMessage message) throws MqttException, UnsupportedEncodingException {
        System.out.println(String.format("[%s] %s", topic, new String(message.getPayload())));
        processor.receive( new String(message.getPayload()));
        processor.print_all();
    }

    public static void main(String[] args) throws MqttException, URISyntaxException {
        Subscriber s = new Subscriber();
    }
}