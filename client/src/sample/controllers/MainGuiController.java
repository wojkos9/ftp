package sample.controllers;

import javafx.application.Application;
import javafx.application.Platform;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Button;
import javafx.scene.control.ChoiceBox;
import javafx.stage.FileChooser;
import javafx.stage.Stage;
import sample.ConnectionManager;
import sample.Main;
import javafx.application.Platform;
import javafx.application.Application;
import javafx.application.Platform;
import javafx.scene.Scene;
import javafx.scene.layout.StackPane;
import javafx.scene.text.Text;
import javafx.stage.Stage;

import java.io.*;
import java.net.Socket;
import java.net.URL;
import java.util.ResourceBundle;

public class MainGuiController extends Application implements Initializable {
    public Button choose_button;
    public javafx.scene.control.Label label_file;
    public javafx.scene.control.TextField dirName;

    private ConnectionManager connectionManager = Main.getConnectionManager();

    private FileChooser fileChooser;

    @FXML
    private ChoiceBox<String> choice_box;

    @FXML
    public void choose_file() {
        Platform.runLater(()->{
            File file = fileChooser.showOpenDialog(
                    choice_box.getScene().getWindow()
            );
            if (file != null) {
                label_file.setText(file.getName());
                System.out.println(file);
            }
        });

    }

    @Override
    public void initialize(URL location, ResourceBundle resources) {
        Platform.runLater(()->{
            fileChooser = new FileChooser();
            updateDirList();
        });
    }

    @FXML
    public void putHandler(ActionEvent actionEvent) throws IOException {
        Platform.runLater(() -> {
            String filename = label_file.getText();
            if (filename.equals("")) return;
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
        });

    }

    public void getHandler(ActionEvent actionEvent) {
        Platform.runLater(() -> {
            String filename = choice_box.getValue();
            if (filename.equals("")) return;
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
        });
    }

    public void createDirectory(ActionEvent actionEvent) throws IOException {
        Platform.runLater(() -> {
            if (dirName.getText().equals("")) return;
            connectionManager.send("MKD " + dirName.getText() + "\r\n");
            dirName.setText("");
            updateDirList();
        });

    }

    public void dirHandler(ActionEvent actionEvent) throws IOException {
        Platform.runLater(() -> {
            updateDirList();
        });
    }

    private void updateDirList() {
        Platform.runLater(() -> {
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
        });
    }

    public void rmdirHandler() {
        Platform.runLater(() -> {
            if (choice_box.getSelectionModel().getSelectedItem() == null || !(choice_box.getSelectionModel().getSelectedItem().contains("/")))
                return;
            connectionManager.send("RMD " + choice_box.getSelectionModel().getSelectedItem() + "\r\n");
            updateDirList();
        });
    }


    public void binaryHandler(ActionEvent actionEvent) {
        Platform.runLater(() -> {
            connectionManager.send("TYPE I\r\n");
        });
    }

    public void asciiHandler(ActionEvent actionEvent) {
        Platform.runLater(() -> {
            connectionManager.send("TYPE A\r\n");
        });
    }

    @Override
    public void start(Stage primaryStage) throws Exception {
    }
}
