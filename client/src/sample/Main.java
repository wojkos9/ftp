package sample;

import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Stage;

import java.net.Socket;

public class Main extends Application {
    public static String ipAddress;
    public static String portNumber;
    public static Socket socket;

    public static void setConnectionManager(ConnectionManager connectionManager) {
        Main.connectionManager = connectionManager;
    }

    private static ConnectionManager connectionManager;
    static boolean socketCreatedProperty;

    static {
        ipAddress = "127.0.0.1";
        portNumber = "2121";
        socketCreatedProperty = false;
    }

    public static ConnectionManager getConnectionManager() {
        return connectionManager;
    }

    @Override
    public void start(Stage primaryStage) throws Exception{
        Parent root = FXMLLoader.load(getClass().getResource("pages/connect.fxml"));
        primaryStage.setTitle("FTP client");
        primaryStage.setScene(new Scene(root, 600   , 400));
        primaryStage.show();
        connectionManager = new ConnectionManager();
    }



    public static void main(String[] args) {
        launch(args);
    }
}
