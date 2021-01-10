package sample.controllers;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.fxml.Initializable;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.ChoiceBox;
import javafx.scene.control.Label;
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
    public static String PATH;
    public Button choose_button;
    public javafx.scene.control.Label label_file;
    public javafx.scene.control.TextField dirName;
    public Label labelPath;

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
        choice_box.getItems().add("current dor");
        choice_box.getItems().add("current dir");
    }

    @FXML
    public void putHandler(ActionEvent actionEvent) throws IOException {
        if (label_file.getText().equals("")) return;
        String msg = label_file.getText() + "\n";
        Main.connectionManager.send(msg);
    }

    public void createDirectory(ActionEvent actionEvent) throws IOException {
        if (dirName.getText().equals("")) return;
        String msg = "mkdir:" + dirName.getText() + "\n";
        Main.connectionManager.send(msg);
        dirName.setText("");
    }

    public void dirHandler(ActionEvent actionEvent) throws IOException {
        Main.connectionManager.send("dir\n");
    }


    public void backwardHandler(ActionEvent actionEvent) throws IOException {
        String text = labelPath.getText();
        int index = choice_box.getSelectionModel().getSelectedIndex();
        if (index >= choice_box.getItems().size() || text.equals("/") || index == -1 || text.lastIndexOf('/') == -1) return;
        String dir = choice_box.getItems().get(index);
        if (labelPath.getText().contains(dir))
            dir =  text.lastIndexOf('/') != 0 ?  labelPath.getText().substring(0, text.lastIndexOf('/')) : labelPath.getText().substring(0, 1);
                    labelPath.setText(dir);
        Main.connectionManager.send("back to " + dir + "\n");
    }

    public void forwardHandler(ActionEvent actionEvent) throws IOException {
        int index = choice_box.getSelectionModel().getSelectedIndex();
        if (index >= choice_box.getItems().size() || index == -1) return;
        String dir = choice_box.getItems().get(index);
        if (labelPath.getText().equals("/")) {
            dir = labelPath.getText() + dir;
        } else {
            dir = labelPath.getText() + '/' + dir;
        }
        labelPath.setText(dir);
        Main.connectionManager.send("forward" + dir + "\n");
    }

    public void getHandler(ActionEvent actionEvent) {
        System.out.println(choice_box.getSelectionModel().getSelectedIndex());
        Main.connectionManager.send("get\n");
    }


    public void rmdirHandler(ActionEvent actionEvent) {
        if (dirName.getText().equals("")) return;
        Main.connectionManager.send("rmdir\n");
    }

    public void binaryHandler(ActionEvent actionEvent) {
        Main.connectionManager.send("binary\n");
    }

    public void asciiHandler(ActionEvent actionEvent) {
        Main.connectionManager.send("ascii\n");
    }
}
