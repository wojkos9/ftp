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
import sample.ConnectionManager;
import sample.Main;

import java.io.*;
import java.net.Socket;
import java.net.URL;
import java.util.ResourceBundle;

public class MainGuiController implements Initializable {
    public Button choose_button;
    public javafx.scene.control.Label label_file;
    public javafx.scene.control.TextField dirName;

    private ConnectionManager connectionManager = Main.getConnectionManager();

    private FileChooser fileChooser;

    @FXML
    private ChoiceBox<String> choice_box;

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
        updateDirList();
    }

    @FXML
    public void putHandler(ActionEvent actionEvent) throws IOException {
        String filename = label_file.getText();
        if (filename.equals("")) return;

        new Thread() {
            @Override
            public void run() {
                super.run();
                Socket dataSocket = connectionManager.forkConnection();
                connectionManager.send("STOR " + filename + "\r\n");
                try (OutputStream out = dataSocket.getOutputStream();
                     FileInputStream fis = new FileInputStream(new File(filename));) {
                    byte[] buffer = new byte[4096];
                    int n;
                    while ((n = fis.read(buffer)) > 0) {
                        out.write(buffer, 0, n);
                    }
                    updateDirList();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                try {
                    dataSocket.close();
                } catch (IOException e) {
                    System.err.println("ERROR " + e.getMessage());
                }
            }
        }.start();
    }

    public void getHandler(ActionEvent actionEvent) {
        String filename = choice_box.getValue();
        if (filename.equals("")) return;
        new Thread() {
            @Override
            public void run() {
                super.run();
                Socket dataSocket = connectionManager.forkConnection();
                connectionManager.send("RETR " + filename + "\r\n");
                try (InputStream in = dataSocket.getInputStream();
                     FileOutputStream fos = new FileOutputStream(new File(filename));) {
                    byte[] buffer = new byte[4096];
                    int n;
                    while ((n = in.read(buffer)) > 0) {
                        fos.write(buffer, 0, n);
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
                try {
                    dataSocket.close();
                } catch (IOException e) {
                    System.err.println("ERROR " + e.getMessage());
                }
            }
        }.start();
    }

    public void createDirectory(ActionEvent actionEvent) throws IOException {
        if (dirName.getText().equals("")) return;
        connectionManager.send("MKD " + dirName.getText() + "\r\n");
        dirName.setText("");
        updateDirList();
    }

    public void dirHandler(ActionEvent actionEvent) throws IOException {
        updateDirList();
    }

    private void updateDirList() {
        new Thread() {
            @Override
            public void run() {
                super.run();
                Socket dataSocket = connectionManager.forkConnection();
                connectionManager.send("LIST\r\n");
                choice_box.getItems().clear();
                try (BufferedReader in = new BufferedReader(new InputStreamReader(dataSocket.getInputStream()));
                ) {
                    String line;
                    while (!(line = in.readLine()).isEmpty()) {
                        choice_box.getItems().add(line.strip());
                        System.out.println(line);
                    }
                    choice_box.getSelectionModel().selectFirst();
                } catch (IOException | NullPointerException ignored) {
                }
                try {
                    dataSocket.close();
                } catch (IOException e) {
                    System.err.println("ERROR " + e.getMessage());
                }

            }
        }.start();
    }

    public void rmdirHandler(ActionEvent actionEvent) {
        if (dirName.getText().equals("")) return;
        connectionManager.send("RMD " + dirName.getText() + "\r\n");
        updateDirList();
    }


    public void binaryHandler(ActionEvent actionEvent) {
        connectionManager.send("TYPE I\r\n");
    }

    public void asciiHandler(ActionEvent actionEvent) {
        connectionManager.send("TYPE A\r\n");
    }
}
