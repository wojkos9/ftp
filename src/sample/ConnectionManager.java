package sample;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.concurrent.TimeUnit;
import java.util.function.Consumer;

public class ConnectionManager {
    private ConnectionThread connThread;

    public ConnectionManager() {
        this.connThread = new ConnectionThread();
        this.connThread.setDaemon(true);
    }

    public Socket getSocket() {
        return connThread.socket;
    }

    public void startConnection() {
        connThread.start();
    }

    public void send(String data) {
        connThread.out.print(data);
        connThread.out.flush();
    }

    public void closeConnection() throws Exception {
        Main.socketCreatedProperty = false;
        if (connThread.socket != null)
            connThread.socket.close();
    }


    private class ConnectionThread extends Thread {

        private Socket socket;
        private PrintWriter out;

        @Override
        public void run() {
            Socket socket = null;
            while (true) {
                try {
                    socket = new Socket(Main.ipAddress, Integer.parseInt(Main.portNumber));
                    if (socket != null) {
                        break;
                    }
                    Thread.sleep(200);
                } catch (IOException | InterruptedException e) {
                    System.out.println("server is down");
                    return;
                }
            }
            try (BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                 PrintWriter out = new PrintWriter(socket.getOutputStream())) {

                this.out = out;
                this.socket = socket;
                Main.socket = socket;
                this.socket.setTcpNoDelay(true);

                while (true) {
                    String cominginText = "";
                    try {
                        cominginText = in.readLine();
                        System.out.println(cominginText);
                    } catch (IOException e) {
                        System.exit(1);
                        break;
                    }
                }
            } catch (Exception e) {
                System.out.println(e);
            }
        }
    }
}
