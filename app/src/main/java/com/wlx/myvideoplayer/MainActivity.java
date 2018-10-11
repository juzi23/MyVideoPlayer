package com.wlx.myvideoplayer;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.wlx.videoplayerlib.WlxVideoPlayer;
import com.wlx.videoplayerlib.listener.OnPreparedListener;

public class MainActivity extends AppCompatActivity {
    private WlxVideoPlayer videoPlayer;
    private SurfaceView sv_surface;
    private Button btn_start;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        initView();

        initPlayer();

        initListener();

    }

    private void initListener() {
        this.btn_start.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
//                videoPlayer.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
                videoPlayer.setSource(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS).getAbsolutePath() + "/aemsjd.mkv");
//                videoPlayer.setSource(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS).getAbsolutePath() + "/HEVC.mp4");
                videoPlayer.prepare();

            }
        });
    }

    private void initView() {
        setContentView(R.layout.activity_main);
        btn_start = findViewById(R.id.btn_start);
        sv_surface = findViewById(R.id.sv_surface);
    }

    private void initPlayer() {
        videoPlayer = new WlxVideoPlayer();
        videoPlayer.setOnPreparedListener(new OnPreparedListener() {
            @Override
            public void OnPrepared() {
                videoPlayer.play();
                videoPlayer.initVideoOutput(sv_surface.getHolder().getSurface());
            }
        });
    }
}
