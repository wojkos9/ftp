package sample;

import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Stage;

import java.net.Socket;

public class Main extends Application {
    static String ipAddress;
    static String portNumber;
    public static Socket socket;
    public static ConnectionManager connectionManager;
    static boolean socketCreatedProperty;

    static {
        ipAddress = "127.0.0.1";
        portNumber = "8080";
        socketCreatedProperty = false;
    }

    @Override
    public void start(Stage primaryStage) throws Exception{
        Parent root = FXMLLoader.load(getClass().getResource("pages/connect.fxml"));
        primaryStage.setTitle("Hello World");
        primaryStage.setScene(new Scene(root, 600   , 400));
        primaryStage.show();
    }







    public static void main(String[] args) {
        launch(args);
    }
}
