package com.cookandroid.test;
//2번째 엑티비티

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageButton;
import android.widget.ImageView;

public class second extends Activity {

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.next);
        final ImageView openi = (ImageView)findViewById(R.id.openimage);
        ImageButton openbtn = (ImageButton) findViewById(R.id.open) ;
        ImageButton closebtn = (ImageButton) findViewById(R.id.close) ;

        closebtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                openi.setImageResource(R.drawable.close);
            }
        });
        openbtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                openi.setImageResource(R.drawable.open);}
        });
    }


    public void myhouse(View view) {
        Intent intent = new Intent(second.this, sub.class);
        startActivity(intent);
    }

}