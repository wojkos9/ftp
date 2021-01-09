package sample.controllers;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.ChoiceBox;
import javafx.stage.Stage;

import javax.print.DocFlavor;
import java.io.IOException;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;

public class Controller {
    @FXML
    private Button connect_button;

    @FXML
    private ChoiceBox<Object> choice_box;


    //    public Controller() throws IOException {
//        FXMLLoader loader = FXMLLoader.load(getClass().getResource("connect.fxml"));
//
//        loader.setController(this);
//    }
    @FXML
    public void test(){
        try {
            Stage old_stage = (Stage) connect_button.getScene().getWindow();
            old_stage.close();
            FXMLLoader fxmlLoader = new FXMLLoader();
            fxmlLoader.setLocation(getClass().getResource("../pages/general-ui.fxml"));
            /*
             * if "fx:controller" is not set in fxml
             * fxmlLoader.setController(NewWindowController);
             */
            Scene scene = new Scene(fxmlLoader.load(), 600, 400);
            Stage stage = new Stage();
            stage.setResizable(false);

            stage.setTitle("New Window");
            stage.setScene(scene);
//            choice_box.getItems().add( "current dir");
//            choice_box.getItems().add( "current dir");
            stage.show();
        } catch (IOException e) {
            Logger logger = Logger.getLogger(getClass().getName());
            logger.log(Level.SEVERE, "Failed to create new Window.", e);
        }
    }
}
