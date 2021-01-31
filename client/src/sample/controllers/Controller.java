package sample.controllers;

import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.fxml.Initializable;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.ChoiceBox;
import javafx.scene.control.TextField;
import javafx.stage.Stage;
import sample.Main;
import sample.ConnectionManager;
import java.io.IOException;
import java.net.URL;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;

public class Controller implements Initializable {
    public TextField ip_adress;
    public TextField port_number;
    private ConnectionManager connectionManager;

    @FXML
    private Button connect_button;

    @FXML
    private ChoiceBox<Object> choice_box;

    @FXML
    public void connectToServer() throws InterruptedException {
        connectionManager = new ConnectionManager();
        Main.setConnectionManager(connectionManager);
        connectionManager.setConnectionParams(ip_adress.getText(), port_number.getText());
        connectionManager.startConnection();
        Thread.sleep(1000);
        connectionManager.send("USER user\r\n");
        connectionManager.send("PASSWORD password\r\n");
        System.out.println(connectionManager.getSocket());
        if(connectionManager.getSocket() != null){
            showMainGuiWindow();
        }
    }

    @FXML
    public void showMainGuiWindow(){
        try {
            Stage old_stage = (Stage) connect_button.getScene().getWindow();
            old_stage.close();
            FXMLLoader fxmlLoader = new FXMLLoader();
            fxmlLoader.setLocation(getClass().getResource("../pages/general-ui.fxml"));
            Scene scene = new Scene(fxmlLoader.load(), 600, 400);
            Stage stage = new Stage();
            stage.setResizable(false);
            stage.setTitle("FTP client");
            stage.setScene(scene);
            stage.show();
        } catch (IOException e) {
            Logger logger = Logger.getLogger(getClass().getName());
            logger.log(Level.SEVERE, "Failed to create new Window.", e);
        }
    }

    @Override
    public void initialize(URL location, ResourceBundle resources) {
        ip_adress.setText(Main.ipAddress);
        port_number.setText(Main.portNumber);
    }
}
