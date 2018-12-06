package com.example.lkh116.hw2;

import android.os.AsyncTask;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    ProgressBar downloadProgress;
    TextView completeText;
    BackgroundProgress backgroundprogress;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        downloadProgress = (ProgressBar)findViewById(R.id.progressBar);
        downloadProgress.setProgress(0);
        completeText = (TextView)findViewById(R.id.textView);

        backgroundprogress = new BackgroundProgress();

        Button startbutton = (Button)findViewById(R.id.startButton);
        Button canclebutton = (Button)findViewById(R.id.cancelButton);

        startbutton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                backgroundprogress = new BackgroundProgress();
                backgroundprogress.execute(100);
            }
        });

        canclebutton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                backgroundprogress.cancel(true);
            }
        });
    }

    private class BackgroundProgress extends AsyncTask<Integer, Integer, Boolean> {
        @Override
        protected Boolean doInBackground(Integer ... totalCount) {
            for (int i = 0; i <= totalCount[0]; i++) {
                publishProgress(i);

                try {
                    Thread.sleep(100);
                }
                catch (InterruptedException e) {
                    return false;
                }
            }

            return true;
        }

        @Override
        protected void onProgressUpdate(Integer ... downloadInfos) {
            int currentCount = downloadInfos[0];

            downloadProgress.setProgress(currentCount);
            completeText.setText("Current value : " + currentCount);

            super .onProgressUpdate(downloadInfos);
        }

        @Override
        protected void onPostExecute(Boolean result) {
            completeText.setText("Download complete!");
            downloadProgress.setProgress(0);

            super .onPostExecute(result);
        }

        @Override
        protected  void onCancelled() {
            completeText.setText("Cancelled");
            downloadProgress.setProgress(0);

            super .onCancelled();
        }

    }

}
