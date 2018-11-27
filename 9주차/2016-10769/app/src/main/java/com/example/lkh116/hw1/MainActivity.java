package com.example.lkh116.hw1;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Button _button = (Button)findViewById(R.id.result);
        _button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                EditText input1 = (EditText) findViewById(R.id.heightInput);
                EditText input2 = (EditText) findViewById(R.id.weightInput);
                int height = Integer.parseInt(input1.getText().toString());
                int weight = Integer.parseInt(input2.getText().toString());

                if (height <= 0 || weight <= 0) {
                    Toast.makeText(getApplicationContext(), "You must enter positive value as input!", Toast.LENGTH_LONG).show();
                } else {
                    double bmi = weight / (Math.pow(height / 100.0, 2));

                    Intent intent = new Intent(MainActivity.this, SubActivity.class);
                    intent.putExtra("bmi", bmi);
                    startActivity(intent);
                }
            }
        });
    }
}
