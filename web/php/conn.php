<?php #DB에 접속하는 코드
    $host = 'localhost'; # ip 입력
    $user = 'root';     # DB 아이디 입력
    $pw = 'test';       # DB 비밀번호 입력
    $dbName = 'test'; # DB 이름 입력
    # mysql에 접속한다.
    $conn =mysqli_connect($host, $user, $pw, $dbName); 
?>