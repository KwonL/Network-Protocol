package com.example.lkh116.hw1;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.ImageView;
import android.widget.TextView;

public class SubActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sub);

        // Get calculated bmi from Main Activity
        Intent intent = getIntent();
        double bmi = intent.getDoubleExtra("bmi", 0.0);

        TextView bmi_view = (TextView)findViewById(R.id.textView);
        String vis_text = String.format("You're bmi is : %f", bmi);
        // Set text as message
        bmi_view.setText(vis_text);

        // Set image with result
        ImageView _image = (ImageView)findViewById(R.id.imageView);
        if (bmi >= 20 && bmi < 25) {
            _image.setImageResource(R.drawable.normal);
        }
        else if (bmi >= 25 && bmi < 30) {
            _image.setImageResource(R.drawable.over);
        }
        else if (bmi >= 30 && bmi <40) {
            _image.setImageResource(R.drawable.obesity);
        }
        else if (bmi >= 40) {
            _image.setImageResource(R.drawable.end);
        }
    }
}
