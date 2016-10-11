#include <Debug.h>
#include "SSLClient.h"
#include "Scraper_lib.h"
#include "Log.h"


char* small_html = "<html xmlns:v=\"urn:schemas-microsoft-com:vml\"
xmlns:o=\"urn:schemas-microsoft-com:office:office\"
xmlns:w=\"urn:schemas-microsoft-com:office:word\"
xmlns:st1=\"urn:schemas-microsoft-com:office:smarttags\"
xmlns=\"http://www.w3.org/TR/REC-html40\">

<head>
<meta http-equiv=Content-Type content=\"text/html; charset=iso-8859-1\">
<meta name=ProgId content=Word.Document>
<meta name=Generator content=\"Microsoft Word 11\">
<meta name=Originator content=\"Microsoft Word 11\">
<link rel=File-List href=\"phys341_files/filelist.xml\">
<link rel=Edit-Time-Data href=\"phys341_files/editdata.mso\">
<!--[if !mso]>
<style>
v\\:* {behavior:url(#default#VML);}
o\\:* {behavior:url(#default#VML);}
w\\:* {behavior:url(#default#VML);}
.shape {behavior:url(#default#VML);}
</style>
<![endif]-->
<title>Physics 3327</title>
<o:SmartTagType namespaceuri=\"urn:schemas-microsoft-com:office:smarttags\"
 name=\"City\"/>
<o:SmartTagType namespaceuri=\"urn:schemas-microsoft-com:office:smarttags\"
 name=\"PlaceType\"/>
<o:SmartTagType namespaceuri=\"urn:schemas-microsoft-com:office:smarttags\"
 name=\"PlaceName\"/>
<o:SmartTagType namespaceuri=\"urn:schemas-microsoft-com:office:smarttags\"
 name=\"place\"/>
<!--[if gte mso 9]><xml>
 <w:WordDocument>
  <w:Zoom>75</w:Zoom>
  <w:SpellingState>Clean</w:SpellingState>
  <w:GrammarState>Clean</w:GrammarState>
  <w:ValidateAgainstSchemas/>
  <w:SaveIfXMLInvalid>false</w:SaveIfXMLInvalid>
  <w:IgnoreMixedContent>false</w:IgnoreMixedContent>
  <w:AlwaysShowPlaceholderText>false</w:AlwaysShowPlaceholderText>
  <w:BrowserLevel>MicrosoftInternetExplorer4</w:BrowserLevel>
 </w:WordDocument>
</xml><![endif]--><!--[if gte mso 9]><xml>
 <w:LatentStyles DefLockedState=\"false\" LatentStyleCount=\"156\">
 </w:LatentStyles>
</xml><![endif]--><!--[if !mso]><object
 classid=\"clsid:38481807-CA0E-42D2-BF39-B33AF135CC4D\" id=ieooui></object>
<style>
st1\\:*{behavior:url(#ieooui) }
</style>
<![endif]-->
<style>
<!--
 /* Style Definitions */
 p.MsoNormal, li.MsoNormal, div.MsoNormal
	{mso-style-parent:\"\";
	margin:0pt;
	margin-bottom:.0001pt;
	mso-pagination:widow-orphan;
	font-size:12.0pt;
	font-family:\"Times New Roman\";
	mso-fareast-font-family:\"Times New Roman\";
	color:black;}
h2
	{mso-margin-top-alt:auto;
	margin-right:0pt;
	mso-margin-bottom-alt:auto;
	margin-left:0pt;
	mso-pagination:widow-orphan;
	mso-outline-level:2;
	font-size:18.0pt;
	font-family:\"Times New Roman\";
	color:black;
	font-weight:bold;}
a:link, span.MsoHyperlink
	{color:blue;
	text-decoration:underline;
	text-underline:single;}
a:visited, span.MsoHyperlinkFollowed
	{color:purple;
	text-decoration:underline;
	text-underline:single;}
p
	{mso-margin-top-alt:auto;
	margin-right:0pt;
	mso-margin-bottom-alt:auto;
	margin-left:0pt;
	mso-pagination:widow-orphan;
	font-size:12.0pt;
	font-family:\"Times New Roman\";
	mso-fareast-font-family:\"Times New Roman\";
	color:black;}
span.SpellE
	{mso-style-name:\"\";
	mso-spl-e:yes;}
span.GramE
	{mso-style-name:\"\";
	mso-gram-e:yes;}
@page Section1
	{size:612.0pt 792.0pt;
	margin:72.0pt 90.0pt 72.0pt 90.0pt;
	mso-header-margin:36.0pt;
	mso-footer-margin:36.0pt;
	mso-paper-source:0;}
div.Section1
	{page:Section1;}
-->
</style>
<!--[if gte mso 10]>
<style>
 /* Style Definitions */
 table.MsoNormalTable
	{mso-style-name:\"Table Normal\";
	mso-tstyle-rowband-size:0;
	mso-tstyle-colband-size:0;
	mso-style-noshow:yes;
	mso-style-parent:\"\";
	mso-padding-alt:0pt 5.4pt 0pt 5.4pt;
	mso-para-margin:0pt;
	mso-para-margin-bottom:.0001pt;
	mso-pagination:widow-orphan;
	font-size:10.0pt;
	font-family:\"Times New Roman\";
	mso-ansi-language:#0400;
	mso-fareast-language:#0400;
	mso-bidi-language:#0400;}
</style>
<![endif]--><!--[if gte mso 9]><xml>
 <o:shapedefaults v:ext=\"edit\" spidmax=\"7170\"/>
</xml><![endif]--><!--[if gte mso 9]><xml>
 <o:shapelayout v:ext=\"edit\">
  <o:idmap v:ext=\"edit\" data=\"1\"/>
 </o:shapelayout></xml><![endif]-->
</head>

<body bgcolor=white lang=EN-US link=blue vlink=purple style='tab-interval:36.0pt'
alink=\"#ff0000\">

<div class=Section1>

<h2>Physics 3327</h2>

<h2>Electricity and Magnetism</h2>

<p class=MsoNormal><b>Lectures: </b>Monday, Wednesday, and Friday, 11.15am-12.05pm,
Rockefeller Hall<span class=GramE>&nbsp; 110</span>. <br>
<b>Section:</b> Friday, 2.30pm-3.20pm, Rockefeller Hall ?. </p>

<p><b>Professor:</b> Itai Cohen, <br>
508 <st1:place w:st=\"on\">Clark</st1:place> Hall, tel. 5-0815, <br>
<a href=\"mailto:ic64@cornell.edu\">ic64@cornell.edu</a>. <br>
<b>Office hours: </b>Tuesday 3:00pm-4:00pm,Wednesday 10:00-11:00am, and by appointment. </p>

<p><span class=GramE><b>Teaching Assistant: </b><span class=SpellE>Shovan</span>
<span class=SpellE>Dutta</span><br>
office, ?, Tel ? <br>
<a href=\"mailto:sd632@cornell.edu\">sd632@cornell.edu</a>.</span><b><br>
Office hours: </b> Day, time, Place </p>

<p><b>Course webpage:</b> <a
href=\"http://cohengroup.ccmr.cornell.edu/courses/2015/physics-3327-electricity-and-magnetism\">http://cohengroup.ccmr.cornell.edu/courses/2015/physics-3327-electricity-and-magnetism</a>
</p>

<div class=MsoNormal align=center style='text-align:center'>

<hr size=2 width=\"100%\" align=center>

</div>

<p><b>Literature:</b> <br>
<i>Classical electromagnetic radiation</i>, <span class=SpellE>Heald and Marion</span>,
Thomson Learning, 1995.<br>
<i style='mso-bidi-font-style:normal'>div grad curl and all that</i>, H. M. Schey </span>, 
Norton and Company, 1973.</p>
<p><b>Additional or alternative literature:</b> <br> 
<i>Introduction to Electrodynamics</i>,
<span class=SpellE> D. J. Griffiths <st1:place w:st=\"on\"><st1:City
 w:st=\"on\">Prentice Hall, 1999.<br
clear=all>
<i>Extra Credit: The boy who harnessed the wind </i>, <span class=SpellE>By William Kamkwamba, Bryan Mealer </span>,
HarperCollins, 2010.<br><p>




<div class=MsoNormal align=center style='text-align:center'>

<hr size=2 width=\"100\%\" align=center>

</div>

<p><b><a href=\"Syllabus.pdf\">Course syllabus and reading assignments</a></b></p>

<p><b><a href=\"homework.htm\">Homework:</a></b> Approximately 11 weekly homework
assignments. <br>
Homework is due on Fridays. </p>

<p><b><a href=\"quizzes.htm\"><span class=SpellE>Quizzes</span>:</a></b> Think of them as practice exams. The first Quiz will be held in class on Sep. 30th.</p>

<div class=MsoNormal align=center style='text-align:center'>

<hr size=2 width=\"100\%\" align=center>

</div>

<p><b><a href=\"Class_Projects.pdf\">Class Projects:</a></b> <!-- this is commented out Class Project will be
described later in the semester.--> </p>

<p><b><a href=\"prelims.htm\">Prelims:</a></b> tentatively October 12th and 
November 21st in class<br>
<b><a href=\"prelims.htm\">Final exam:</a></b> date?</p>

<p>Makeup exams/quizes will be oral unless the original exam date conflicts with a
religious holiday or students have made alternative arrangement with the
instructor. </p>

<p><b>Final grade:</b> based on homework (20%), quizzes and
participation (5%), one class project (25%), two prelims (50%) and a possible makeup final
examination (15%). </p>

<p><b>Academic Integrity:</b> Each student in this course is expected to abide
by the Cornell University Code of Academic Integrity. You are allowed to
discuss the subject and the homework problems with others. However, you should
write up the homework by yourself and acknowledge all sources including a list
of people you worked with. Moreover, only you will be held responsible for all
the content you submit. </p>

<p><a href=\"http://cuinfo.cornell.edu/Academic/AIC.html\">http://cuinfo.cornell.edu/Academic/AIC.html</a></p>

<p><b>Socratic Teaching Method:</b> Class participation is mandatory. Everyone
is expected to participate in discussions relating to reading materials,
homework, exams and lectures. </p>

<p><b>Guaranteed Recipe for Success:</b> <br>
1) Take notes during lecture and sections. <br>
2) After each lecture but before the next lecture review your notes. Identify
the parts you do not understand. <br>
3) Come to each lecture and discussion section with specific questions. <br>
4) Keep up with the reading so that you have some familiarity with each topic
prior to hearing about it in the lecture <br>
5) Find at least one &quot;partner&quot; in the class with whom you can meet at
least once or twice a week to discuss materials from the lectures, the reading
assignments and the homework. <br>
6) Take the homework assignment seriously. Do not try to do the whole
assignment the night before it is due. Some version of the homework questions
will appear on the exams. <br>
&nbsp; <br>
&nbsp; </p>

</div>

</body>

</html>";



int test_get_page_on_sll(){
	//We want to see test_get_page_on_sll fail in cases where there is a timeout
	http://uuuuuu.lassp.cornell.edu/courses/physics_3318_spring_2016
	int buf_size = 100*1024;
	char* buf = (char*) malloc(buf_size);
	get_page_on_ssl("http://uuuuuu.lassp.cornell.edu", "/courses/physics_3318_spring_2016", NULL, 0, buf, buf_size);

	//Case: Test parsing a simple webpage that does not contain content-length
	char* buf_small = (char*)malloc(buf_size);
	for(int i = 0; i < 10000; i ++){
		get_page_on_ssl("http://cohengroup.ccmr.cornell.edu","/courses/phys3327/phys3327.htm", \
			NULL, 0, buf_small);

		

	}

	//Case: Test parsing a webpage that does not contain content-size field
	//https://www.cs.cornell.edu/undergrad/csmajor/vectors
	char* buf2 = (char*)malloc(buf_size);
	for (int i = 0; i < 10000; i++){
		get_page_on_ssl("https://www.cs.cornell.edu", "/undergrad/csmajor/vectors", NULL, 0, buf2, buf_size);
	}

	char* buf3 = (char*)malloc(buf_size);
	for (int i = 0; i < 10000; i++){
		get_page_on_ssl("https://en.wikipedia.org","/wiki/Chunked_transfer_encoding", NULL, 0, buf3, buf_size);
	}



	return 0;
}


i