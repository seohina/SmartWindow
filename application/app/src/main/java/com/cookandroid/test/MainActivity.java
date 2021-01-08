package com.cookandroid.test;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.View;
import android.widget.EditText;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLEncoder;


public class MainActivity extends Activity {


    EditText pas, usr;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        usr = (EditText) findViewById(R.id.username);
        pas = (EditText) findViewById(R.id.password);
    }

    public void loginBtn(View view) {
        String user = usr.getText().toString();
        String pass = pas.getText().toString();
        String type = "login";
        background bg = new background(this);
        bg.execute(type, user,pass);

    }

    public class background extends AsyncTask<String, Void, String> {
        // doInBackground의 매개값이 문자열 배열인데요. 보낼 값이 여러개일 경우를 위해 배열로 합니다.
        AlertDialog dialog;
        Context context;

        public background(Context context) {
            this.context = context;
        }

        @Override
        protected void onPreExecute() {    //background 시작 전에 ui 구성
            dialog = new AlertDialog.Builder(context).create();
            //dialog.setTitle("Login status");
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(String result) {// background 작업이 끝난 후 ui 구성
            dialog.setMessage(result);
            dialog.dismiss();
            if (result.equals("login success")) {
                Toast.makeText(context, "로그인 성공", Toast.LENGTH_LONG).show();
                Intent intent = new Intent(MainActivity.this, second.class);
                startActivity(intent);
            }
            else {
                Toast.makeText(context, "로그인 실패", Toast.LENGTH_LONG).show();
            }
        }

        @Override
        protected void onProgressUpdate(Void... values) {

            super.onProgressUpdate(values);
        }

        @Override
        protected String doInBackground(String... paramas) { //background 작업진행
            String result = ""; //요청 결과를 저장할 변수
            String type = paramas[0];

            String connstr = "http://192.168.137.61/login4.php";

            if (type.equals("login")) {
                try {
                    String user_name = paramas[1];
                    String password = paramas[2];
                    URL url = new URL(connstr);
                    HttpURLConnection http = (HttpURLConnection) url.openConnection(); //url접속
                    http.setRequestMethod("POST");//데이터를 POST 방식으로 전송합니다.
                    http.setDoInput(true);
                    http.setDoOutput(true);

                    OutputStream ops = http.getOutputStream();
                    BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(ops, "UTF-8"));
                    String data = URLEncoder.encode("user_name", "UTF-8") + "=" + URLEncoder.encode(user_name, "UTF-8");


                    writer.write(data);//OutputStreamWriter에 담아 전송
                    writer.flush();//출력 스트림을 비운다하고 버퍼링된 모든 출력
                    writer.close();//출력 스트림을 닫고 모든 시스템 자원을 해제
                    ops.close();

                    //php와 통신이 정상적으로 되었을 때 할 코드
                    InputStream ips = http.getInputStream();
                    BufferedReader reader = new BufferedReader(new InputStreamReader(ips, "ISO-8859-1"));
                    //php에서 보낸 값을 받는다

                    String line = "";

                    //라인을 받아와 합친다.
                    while ((line = reader.readLine()) != null) {
                        result += line;
                    }
                    reader.close();
                    ips.close();
                    http.disconnect();
                    return result;

                } catch (MalformedURLException e) {//for url
                    result = e.getMessage();
                } catch (IOException e) {//for openConnection().
                    result = e.getMessage();
                }
            }

            return result;
        }
    }

}


