package sample;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;

public class ConnectionManager {
    private ConnectionThread connThread;
    private String ipAddress=Main.ipAddress;
    private String portNumber=Main.portNumber;

    public ConnectionManager() {
        this.connThread = new ConnectionThread();
        this.connThread.setDaemon(true);
    }

    public void setConnectionParams(String ip, String port) {
        ipAddress = ip;
        portNumber = port;
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

    public Socket forkConnection() {
        return connThread.forkConnection();
    }


    private class ConnectionThread extends Thread {

        private Socket socket;
        private PrintWriter out;

        @Override
        public void run() {
            Socket socket = null;
            while (true) {
                try {
                    socket = new Socket(ipAddress, Integer.parseInt(portNumber));
                    if (socket.isConnected()) {
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

        public Socket
        forkConnection() {
            String cmd = "PORT " + socket.getLocalAddress().toString().substring(1).replace(".", ",");
            ServerSocket sock;
            Socket dataSock;
            try {
                sock = new ServerSocket(0);
                int port = sock.getLocalPort();
                System.out.println("PORT " + port);
                cmd += ","+(port / 256)+","+(port % 256)+"\r\n";
                out.print(cmd);
                out.flush();
                dataSock = sock.accept();
                System.out.printf("Socket accepted");
                return dataSock;
            } catch (IOException e) {
                e.printStackTrace();
            }
            return null;
        }
    }
}
