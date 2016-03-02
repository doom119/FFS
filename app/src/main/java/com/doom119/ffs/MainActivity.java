package com.doom119.ffs;

import android.app.Activity;
import android.os.Bundle;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.View;

public class MainActivity extends Activity
{
    public static final String TAG = MainActivity.class.getSimpleName();

    private SurfaceView mSufaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mSufaceView = (SurfaceView)findViewById(R.id.surface_view);
    }

    @Override
    protected void onResume()
    {
        super.onResume();
        //SDL2.nativeResume();
    }

    @Override
    protected void onPause()
    {
        super.onPause();
        //SDL2.nativePause();
    }

    @Override
    protected void onDestroy()
    {
        super.onDestroy();
//        FFS.close();
    }

    public void init(View v)
    {
        FFS.init(mSufaceView);
        mSufaceView.setVisibility(View.VISIBLE);
    }

    public void open(View v)
    {
        String videoPath = "/mnt/sdcard/2.mp4";
        FFS.open(videoPath);
    }

    public void play(View v)
    {
        FFS.play();
    }

    public void close(View v)
    {
        mSufaceView.setVisibility(View.GONE);
        FFS.close();
    }
}
