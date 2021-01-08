<?php  # 데이터베이스에 저장된 측정시간, 온도, 습도를 php로 출력

$link=mysqli_connect("localhost","root","test", "test" );  #mysql 연결
if (!$link)   #연결이 없을 때 오류 메시지 출력
{  
   echo "MySQL 접속 에러 : ";
   echo mysqli_connect_error();
   exit();  
}  
mysqli_set_charset($link,"utf8");  

$sql="select * from sensor ORDER BY collect_time DESC limit 1";
# sensor 테이블에서 collect_time을 내림차순으로 정렬하여 하나만 출력

$result=mysqli_query($link,$sql);  # 쿼리 실행
$data = array();   
if($result){    
   while($row=mysqli_fetch_array($result)){  #결과를 배열로 변환
       array_push($data, 
           array('collect_time'=>$row[1],
           'temperature'=>$row[2],
           'humiditiy'=>$row[3]
       ));   #측정시간, 온도, 습도를 불러와 배열로 변환한다.
   }

header('Content-Type: application/json; charset=utf8');
$json=json_encode(array("webnautes"=>$data),JSON_PRETTY_PRINT+JSON_UNESCAPED_UNICODE);  # 배열을 JSON 형식으로 변환
echo $json;
}  
else{  
   echo "error : "; 
   echo mysqli_error($link);
} 

mysqli_close($link);  #mysql 연결 종료
  
?>