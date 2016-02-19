package com.doom119.ffs;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;

public class MainActivity extends AppCompatActivity
{

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    public void init(View v)
    {
        FFS.init();
    }

    public void open(View v)
    {
        String videoPath = "/mnt/sdcard/1.mp4";
        FFS.open(videoPath);
    }

    public void decode(View v)
    {
        FFS.decode();
    }

    public void close(View v)
    {
        FFS.close();
    }
}
