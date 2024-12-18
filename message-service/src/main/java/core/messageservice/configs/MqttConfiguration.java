package core.messageservice.configs;

import core.messageservice.entity.WaterLevel;
import core.messageservice.repository.WaterSensorRepository;
import jakarta.annotation.PostConstruct;
import jakarta.annotation.PreDestroy;
import lombok.AllArgsConstructor;
import lombok.experimental.FieldDefaults;
import lombok.experimental.NonFinal;
import org.eclipse.paho.client.mqttv3.*;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

import java.time.LocalDateTime;
import java.util.Date;

@Configuration
@FieldDefaults(level = lombok.AccessLevel.PRIVATE, makeFinal = true)
public class MqttConfiguration {
    @NonFinal
    @Autowired
    WaterSensorRepository waterSensorRepository;

    @NonFinal
    @Value("${mqtt.broker.url}")
    String brokerUrl;

    @NonFinal
    @Value("${mqtt.client.id}")
    String clientId;

    @NonFinal
    @Value("${mqtt.client.topic.water}")
    String waterTopic;


    @Bean
    public MqttClient mqttClient() throws MqttException {
        MemoryPersistence persistence = new MemoryPersistence();
        return new MqttClient(brokerUrl, clientId, persistence);
    }

    @Component
    public class MqttMessageReceiver implements MqttCallback {
        private final MqttClient mqttClient;
        private static final int QOS = 1; // At least once delivery

        public MqttMessageReceiver(MqttClient mqttClient) {
            this.mqttClient = mqttClient;
        }

        @PostConstruct
        public void init() {
            try {
                // Set callback to handle connection events
                mqttClient.setCallback(this);

                // Connect and subscribe
                connectAndSubscribe();
            } catch (Exception e) {
                System.out.println("Error initializing MQTT client: " + e);
            }
        }

        private void connectAndSubscribe() throws MqttException {
            if (!mqttClient.isConnected()) {
                MqttConnectOptions options = new MqttConnectOptions();
                options.setCleanSession(false);
                options.setAutomaticReconnect(true);
                options.setConnectionTimeout(30); // 30 seconds connection timeout

                mqttClient.connect(options);
                System.out.println("Connected to MQTT broker");
            }

            // Subscribe to the topic with QoS
            mqttClient.subscribe(waterTopic, QOS);
            System.out.println("Subscribed to topic: " + waterTopic);
        }

        @Override
        public void messageArrived(String topic, MqttMessage message) throws Exception {
            try {
                String receivedMessage = new String(message.getPayload());
                processReceivedMessage(topic, receivedMessage);
            } catch (Exception e) {
                System.out.println("Error processing received message: " + e);
            }
        }

        private void processReceivedMessage(String topic, String message) {
            try {
                System.out.println("Received message from topic: " + topic);
                System.out.println("Message content: " + message);

                // Parse water level
                int waterLevel = Integer.parseInt(message);

                WaterLevel water = new WaterLevel(waterLevel, new Date());

                waterSensorRepository.save(water);

                // Log the water level
                System.out.println("Water level: " + waterLevel);


            } catch (NumberFormatException e) {
                System.out.println("Invalid water level format: " + message);
            }
        }

        @Override
        public void connectionLost(Throwable cause) {
            System.out.println("MQTT connection lost");
            try {
                // Attempt to reconnect
                connectAndSubscribe();
            } catch (MqttException e) {
                System.out.println("Failed to reconnect to MQTT broker: " + e);
            }
        }

        @Override
        public void deliveryComplete(IMqttDeliveryToken token) {
            // This method is called when a message is successfully delivered
            System.out.println("Message delivery complete");
        }

        @PreDestroy
        public void cleanup() {
            try {
                if (mqttClient != null && mqttClient.isConnected()) {
                    mqttClient.disconnect();
                    System.out.println("MQTT client disconnected");
                }
            } catch (MqttException e) {
                System.out.println("Error disconnecting MQTT client: " + e);
            }
        }

//        @Scheduled(fixedRate = 60000) // 1 minute interval
//        public void readWaterLevel() {
//            try {
//                // Your code to read the water level
//                System.out.println("Reading water level...");
//                // Simulate reading water level
//                String message = "100"; // Replace with actual reading logic
//                processReceivedMessage(waterTopic, message);
//            } catch (Exception e) {
//                System.out.println("Error reading water level: " + e);
//            }
//        }
    }
}