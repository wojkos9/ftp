package sample.controllers;

import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.fxml.Initializable;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.ChoiceBox;
import javafx.stage.FileChooser;
import javafx.stage.Stage;

import java.awt.*;
import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;

public class MainGuiController implements Initializable {

    public Button choose_button;
    public javafx.scene.control.Label label_file;

    private FileChooser fileChooser;
    private Desktop desktop;
    @FXML
    private Button connect_button;

    @FXML
    public Label file_label;


    @FXML
    private ChoiceBox<String> choice_box;

    //    public Controller() throws IOException {
//        FXMLLoader loader = FXMLLoader.load(getClass().getResource("connect.fxml"));
//
//        loader.setController(this);
//    }


    @FXML
    public void show() {
        System.out.println(choice_box.getItems() + "kek");

    }

    @FXML
    public void choose_file() {
        File file = fileChooser.showOpenDialog(
                choice_box.getScene().getWindow()
        );
        if (file != null) {
            label_file.setText(file.getName());
            System.out.println(file);
        }
    }

//    private void openFile(File file) {
//        try {
//            desktop.open(file);
//        } catch (IOException ex) {
//            Logger.getLogger(
//                    MainGuiController.class.getName()).log(
//                    Level.SEVERE, null, ex
//            );
//        }
//    }

    @FXML
    public void test() {
        try {


            Stage old_stage = (Stage) connect_button.getScene().getWindow();
            old_stage.close();
            FXMLLoader fxmlLoader = new FXMLLoader();
            fxmlLoader.setLocation(getClass().getResource("pages/general-ui.fxml"));
            /*
             * if "fx:controller" is not set in fxml
             * fxmlLoader.setController(NewWindowController);
             */

            Scene scene = new Scene(fxmlLoader.load(), 600, 400);
            Stage stage = new Stage();
            stage.setTitle("New Window");
            stage.setScene(scene);
            stage.show();

        } catch (IOException e) {
            Logger logger = Logger.getLogger(getClass().getName());
            logger.log(Level.SEVERE, "Failed to create new Window.", e);
        }
    }

    @Override
    public void initialize(URL location, ResourceBundle resources) {

        desktop = Desktop.getDesktop();
        fileChooser = new FileChooser();
        choice_box.getItems().add("current dir");
        choice_box.getItems().add("current dir");
    }
}
