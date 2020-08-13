package com.android.ffmpegturorial;

import androidx.appcompat.app.AppCompatActivity;

import android.media.MediaPlayer;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {

    SimplePlayer simplePlayer;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        SurfaceView surfaceView = findViewById(R.id.surfaceView);
        initSimplePlayer(surfaceView);
        findViewById(R.id.tvPrepare).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                prepareFFmpeg();
            }
        });
    }

    private void initSimplePlayer(SurfaceView surfaceView){
        simplePlayer = new SimplePlayer();
        TextView tv = findViewById(R.id.tvVersion);
        tv.setText(simplePlayer.getVersionNative());
        simplePlayer.setOnPrepareListener(new SimplePlayer.OnPrepareListener() {
            @Override
            public void onPrepare() {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(MainActivity.this, "开始播放", Toast.LENGTH_SHORT).show();
                    }
                });
                simplePlayer.startPlay();
            }

            @Override
            public void onError(int errorCode) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(MainActivity.this, "播放出错", Toast.LENGTH_SHORT).show();
                    }
                });
            }
        });

        simplePlayer.setSurfaceView(surfaceView);
    }

    public void prepareFFmpeg() {
        String url = "http://ivi.bupt.edu.cn/hls/cctv1hd.m3u8";
        simplePlayer.prepare(url);
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onStop() {
        super.onStop();
        simplePlayer.stop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        simplePlayer.release();
    }
}
