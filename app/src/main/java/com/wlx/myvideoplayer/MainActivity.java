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

import com.lidroid.xutils.ViewUtils;
import com.lidroid.xutils.view.annotation.ViewInject;
import com.wlx.videoplayerlib.WlxVideoPlayer;
import com.wlx.videoplayerlib.listener.OnPreparedListener;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {
    private WlxVideoPlayer videoPlayer;
    @ViewInject(R.id.sv_surface)
    private SurfaceView sv_surface;
    @ViewInject(R.id.btn_start)
    private Button btn_start;
    @ViewInject(R.id.btn_pause)
    private Button btn_pause;
    @ViewInject(R.id.btn_resume)
    private Button btn_resume;
    @ViewInject(R.id.btn_stop)
    private Button btn_stop;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        initView();

        initPlayer();

        initListener();

    }

    private void initListener() {
        btn_start.setOnClickListener(this);
        btn_pause.setOnClickListener(this);
        btn_resume.setOnClickListener(this);
        btn_stop.setOnClickListener(this);
    }

    private void initView() {
        setContentView(R.layout.activity_main);
        ViewUtils.inject(this);
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

    @Override
    public void onClick(View v) {
        switch(v.getId()) {
            case R.id.btn_start:
                videoPlayer.setSource(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS).getAbsolutePath() + "/aemsjd.mkv");
                videoPlayer.prepare();
                break;
            case R.id.btn_pause:
                videoPlayer.pause();
                break;
            case R.id.btn_resume:
                videoPlayer.resume();
                break;
            case R.id.btn_stop:
                videoPlayer.stop();
                break;
            default:
                break;
        }
    }
}
