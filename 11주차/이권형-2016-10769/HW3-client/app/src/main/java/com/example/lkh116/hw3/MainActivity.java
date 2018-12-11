package com.example.lkh116.hw3;

import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import org.w3c.dom.Text;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.HttpURLConnection;
import java.net.Socket;
import java.net.URL;
import java.security.cert.CertificateParsingException;
import java.util.concurrent.ThreadLocalRandom;

public class MainActivity extends AppCompatActivity {

    EditText inputText;
    EditText serverText;
    TextView resultText;
    Handler handler = new Handler();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        inputText = (EditText) findViewById(R.id.editText);
        resultText = (TextView) findViewById(R.id.textView);
        serverText = (EditText) findViewById(R.id.editText2);

        Button button = (Button) findViewById(R.id.button);
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                int input_num;
                try {
                    input_num = Integer.parseInt(inputText.getText().toString());
                }
                catch (Exception e) {
                    Toast.makeText(MainActivity.this, "Input must be three digit", Toast.LENGTH_LONG).show();
                    input_num = 0;
                }
                if(!inputText.getText().toString().equals("000") && (input_num >= 1000 || input_num < 100)) {
                    Toast.makeText(MainActivity.this, "Input must be three digit", Toast.LENGTH_LONG).show();
                }
                else {
                    ConnetThread run_thread = new ConnetThread();
                    run_thread.start();
                }
            }
        });
    }

    class ConnetThread extends Thread {
        public void run() {
            String num = inputText.getText().toString();
            String serverAddr = serverText.getText().toString();
            try {
                Socket aSocket = new Socket(serverAddr, 11001);

                ObjectOutputStream outstream = new ObjectOutputStream(aSocket.getOutputStream());
                outstream.writeObject(num);
                outstream.flush();

                ObjectInputStream instream = new ObjectInputStream(aSocket.getInputStream());
                final Object response = instream.readObject();

                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        resultText.setText(response.toString());
                    }
                });
                aSocket.close();
            }
            catch (Exception ex) {
                ex.printStackTrace();
            }
        }
    }
}
