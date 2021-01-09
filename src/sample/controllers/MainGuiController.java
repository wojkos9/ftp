package sample.controllers;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.fxml.Initializable;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.ChoiceBox;
import javafx.stage.FileChooser;
import javafx.stage.Stage;
import sample.Main;

import java.awt.*;
import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;
import java.net.URL;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;

import static sample.Main.socket;

public class MainGuiController implements Initializable {

    public Button choose_button;
    public javafx.scene.control.Label label_file;
    public javafx.scene.control.TextField dirName;

    private FileChooser fileChooser;

    @FXML
    private ChoiceBox<String> choice_box;


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

    @Override
    public void initialize(URL location, ResourceBundle resources) {
        fileChooser = new FileChooser();
        choice_box.getItems().add("current dir");
        choice_box.getItems().add("current dir");
    }

    @FXML
    public void putHandler(ActionEvent actionEvent) throws IOException {
        if(label_file.getText().equals("")) return;
        String msg = label_file.getText() + "\n";
        Main.connectionManager.send(msg);
    }

    public void createDirectory(ActionEvent actionEvent) throws IOException {
        if(dirName.getText().equals("")) return;
        String msg = "mkdir:" + dirName.getText() + "\n";
        Main.connectionManager.send(msg);
        dirName.setText("");
    }

    public void dirHandler(ActionEvent actionEvent) throws IOException {
        Main.connectionManager.send("dir\n");
    }


    public void backwardHandler(ActionEvent actionEvent) throws IOException {
        Main.connectionManager.send("back\n");
    }

    public void getHandler(ActionEvent actionEvent) {
        Main.connectionManager.send("get\n");
    }

    public void forwardHandler(ActionEvent actionEvent) throws IOException {
        Main.connectionManager.send("forward\n");
    }

    public void rmdirHandler(ActionEvent actionEvent) {
        if(dirName.getText().equals("")) return;
        Main.connectionManager.send("rmdir\n");
    }

    public void binaryHandler(ActionEvent actionEvent) {
        Main.connectionManager.send("binary\n");
    }

    public void asciiHandler(ActionEvent actionEvent) {
        Main.connectionManager.send("ascii\n");
    }
}
