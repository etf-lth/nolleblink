#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
//gcc -Wall -g -o tutorial maingui.c `pkg-config --cflags --libs gtk+-2.0` -export-dynamic
//gtk-builder-convert mainwin.glade mainwin.xml
GtkWidget *entry1;
 GtkWidget *entry2;
 GtkWidget *entry3;
 GtkWidget *entry4;
 GtkWidget *entry5;
 GtkWidget *entry6;
 GtkWidget *chkbtn;
 GtkWidget *statusbar;
// char sCmd[83];
unsigned char inptext[50];
 const gchar *entry_text;
 const gchar *tmptext;
 int getadr();
 int gettimes();
 void initserial();
 void openfile();
 void readlast();
 void closefile();
 void writenewnolla();
int getnewadr();
 int getid();
 int getinfrate();
 int getcurerate();
 void fixcmd();
 unsigned int inptextlength;
 int fd;
 static const char filename[] = "nollor";
 FILE *file;
 char line [ 128 ]; /* or other suitable maximum line size */
void on_window_destroy (GtkObject *object, gpointer user_data){
	gtk_main_quit ();
}


void  on_button1_clicked (GtkObject *object, gpointer user_data){ // write flash
	
	char sCmd[83];
	
	int i;
	
	fixcmd();
	
	sprintf(sCmd,"$*");
	sCmd[0+2] = 0;
	
	if(GTK_TOGGLE_BUTTON (chkbtn)->active){ // broadcast
		sCmd[1+2] = 0;
		sCmd[2+2] = 0;
	}
	else{
		sCmd[1+2] = (getadr() >> 8) & 0xff;
		sCmd[2+2] = getadr() & 0xff;
	}
	
	
	sCmd[3+2] = 0; 
	sCmd[4+2] = 5; // senderid 5
	sCmd[5+2] = 0;
	sCmd[6+2] = 5;
	sCmd[7+2] = 7; // type 7 write text to flash
	sCmd[8+2] = inptextlength;
	for(i = 0; i < inptextlength; i++){
		sCmd[9+i+2] = inptext[i];
	}
	printf("text: %s length: %d",inptext,inptextlength);
	write(fd,sCmd,83);
	
	
	
	
	
	
	/*
	
	
	char sCmd[83];
	
	
	entry_text = gtk_entry_get_text (GTK_ENTRY (entry1));
	printf ("Entry contents: %s\n", entry_text);
	printf ("sending\n");

	
	sprintf(sCmd,"$*23456%s  ",entry_text);
	sCmd[2] = 5; // pkttype textmessage
	sCmd[3] = getadr();
	
	sCmd[4] = 1; // senderaddress
	sCmd[5] = gettimes(); // this should be the address, hopefully
	sCmd[6] = (unsigned char)strlen(entry_text)+1;
	printf("adr: %i",sCmd[3]);
	fixcmd();
	write(fd,sCmd,83);*/
	
}



void  on_button2_clicked (GtkObject *object, gpointer user_data){ // scroll
	int i;
	char test[50];
	char sCmd[83];
	fixcmd();
	
	sprintf(sCmd,"$*");
	sCmd[0+2] = 0;
	if(GTK_TOGGLE_BUTTON (chkbtn)->active){ // broadcast
		sCmd[1+2] = 0;
		sCmd[2+2] = 0;
	}
	else{
		sCmd[1+2] = (getadr() >> 8) & 0xff;
		sCmd[2+2] = getadr() & 0xff;
	}
	sCmd[3+2] = 0; 
	sCmd[4+2] = 5; // senderid 5
	sCmd[5+2] = 0;
	sCmd[6+2] = 5;
	sCmd[7+2] = 2; // type 2 send txt
	sCmd[8+2] = inptextlength+1;
	sCmd[9+2] = gettimes();
	for(i = 0; i < inptextlength; i++){
		sCmd[10+i+2] = inptext[i];
	}
	printf("text: %s length: %d",inptext,inptextlength);
	write(fd,sCmd,83);
}
void  on_button3_clicked (GtkObject *object, gpointer user_data){
	printf("Button3 \n");	
}
unsigned int oldcrc = 0;
void  on_button4_clicked (GtkObject *object, gpointer user_data){ // initdisp
	int i;
	char sCmd[83];
	unsigned int curcrc = 0;
	int curid;
	
	// init new id -----------------------
	fixcmd();
	for(i = 0; i < inptextlength; i++){
		curcrc += inptext[i];
	}
	if(curcrc != oldcrc){
	curid = getid()+1;
	}
	else{
		curid = getid();
	}
	sprintf(sCmd,"$*");
	sCmd[0+2] = 0;
	sCmd[1+2] = 0x27; // id of new
	sCmd[2+2] = 0xf; // id of new
	sCmd[3+2] = 0; 
	sCmd[4+2] = 5; // senderid 5
	sCmd[5+2] = 0;
	sCmd[6+2] = 5;
	sCmd[7+2] = 10; // type 2 send txt
	sCmd[8+2] = 2;
	sCmd[9+2] = (curid >> 8) & 0xff;
	sCmd[10+2] = (curid) & 0xff;
	write(fd,sCmd,83);
	
	usleep(100000);
	usleep(100000);
	usleep(100000);
	usleep(100000);
	usleep(100000);
	//
	
	
	sprintf(sCmd,"$*");
	sCmd[0+2] = 0;
	
	sCmd[1+2] = (curid >> 8) & 0xff;
	sCmd[2+2] = curid & 0xff;
	printf("curid: %d",curid);
	
	sCmd[3+2] = 0; 
	sCmd[4+2] = 5; // senderid 5
	sCmd[5+2] = 0;
	sCmd[6+2] = 5;
	sCmd[7+2] = 7; // type 7 write text to flash
	sCmd[8+2] = inptextlength;
	for(i = 0; i < inptextlength; i++){
		sCmd[9+i+2] = inptext[i];
	}
	printf("text: %s length: %d",inptext,inptextlength);
	write(fd,sCmd,83);
	
	if(curcrc != oldcrc){
		writenewnolla();
		oldcrc = curcrc;
	}
	
	
	
// 	entry_text = gtk_entry_get_text (GTK_ENTRY (entry1));
// 	
// 	sprintf(sCmd,"$*23456%s  ",entry_text);
// 	sCmd[2] = 3; // pkttype textmessage
// 	sCmd[3] = 10; //always 10 when initing
// 	sCmd[4] = 1;
// 	sCmd[5] = getid()+1; // this should be the address, hopefully
// 	sCmd[6] = (unsigned char)strlen(entry_text)+1;
// 	fixcmd();
// 	write(fd,sCmd,83);
// 	writenewnolla();
}
void  on_button5_clicked (GtkObject *object, gpointer user_data){ // sync
	char sCmd[83];
	
	sprintf(sCmd,"$*");
	sCmd[0+2] = 0;
	if(GTK_TOGGLE_BUTTON (chkbtn)->active){ // broadcast
		sCmd[1+2] = 0;
		sCmd[2+2] = 0;
	}
	else{
		sCmd[1+2] = (getadr() >> 8) & 0xff;
		sCmd[2+2] = getadr() & 0xff;
	}
	sCmd[3+2] = 0; 
	sCmd[4+2] = 5; // senderid 5
	sCmd[5+2] = 0;
	sCmd[6+2] = 5;
	sCmd[7+2] = 4; // type 4 sync
	sCmd[8+2] = 0;
	
	write(fd,sCmd,83);
	printf("Sync \n");	
}
void  on_button6_clicked (GtkObject *object, gpointer user_data){ // e-virus
	
	
	
	char sCmd[83];
	
	sprintf(sCmd,"$*");
	sCmd[0+2] = 0;
	if(GTK_TOGGLE_BUTTON (chkbtn)->active){ // broadcast
		sCmd[1+2] = 0;
		sCmd[2+2] = 0;
	}
	else{
		sCmd[1+2] = (getadr() >> 8) & 0xff;
		sCmd[2+2] = getadr() & 0xff;
	}
	sCmd[3+2] = 0; 
	sCmd[4+2] = 5; // senderid 5
	sCmd[5+2] = 0;
	sCmd[6+2] = 5;
	sCmd[7+2] = 9; // type 9 virus
	sCmd[8+2] = 2;
	sCmd[9+2] = getinfrate();
	sCmd[10+2] = getcurerate();
	
	write(fd,sCmd,83);
	
	
	
	
	
	
	
// 	char sCmd[83];
// 	sprintf(sCmd,"$*23456");
// 	printf("inf is: %i  cur is: %i\n",getinfrate(),getcurerate());	
// 	sCmd[2] = 9; // pkttype show id
// 	if(GTK_TOGGLE_BUTTON (chkbtn)->active){
// 		sCmd[3] = 0;
// 	}
// 	else
// 		sCmd[3] = getadr();
// 	sCmd[4] = 1;
// 	sCmd[6] = 3;
// 	sCmd[9] = getinfrate();
// 	sCmd[10] = getcurerate();
// 	sCmd[11] = 33; // always gets infected
// 	write(fd,sCmd,83);
	
}
void  on_button7_clicked (GtkObject *object, gpointer user_data){ // show id
	char sCmd[83];
	
	sprintf(sCmd,"$*");
	sCmd[0+2] = 0;
	if(GTK_TOGGLE_BUTTON (chkbtn)->active){ // broadcast
		sCmd[1+2] = 0;
		sCmd[2+2] = 0;
	}
	else{
		sCmd[1+2] = (getadr() >> 8) & 0xff;
		sCmd[2+2] = getadr() & 0xff;
	}
	sCmd[3+2] = 0; 
	sCmd[4+2] = 5; // senderid 5
	sCmd[5+2] = 0;
	sCmd[6+2] = 5;
	sCmd[7+2] = 3; // type 4 sync
	sCmd[8+2] = 0;
	
	write(fd,sCmd,83);
}
void  on_button8_clicked (GtkObject *object, gpointer user_data){ // set address on a nolleblink without adding it to database
	
	entry_text = gtk_entry_get_text (GTK_ENTRY (entry1));
	
	char sCmd[83];
	sprintf(sCmd,"$*");
	sCmd[0+2] = 0;
	sCmd[1+2] = 0;//(getadr() >> 8) & 0xff;
	sCmd[2+2] = 0;//getadr() & 0xff;
	sCmd[3+2] = 0; 
	sCmd[4+2] = 5; // senderid 5
	sCmd[5+2] = 0;
	sCmd[6+2] = 5;
	sCmd[7+2] = 10; // type 2 send txt
	sCmd[8+2] = 2;
	sCmd[9+2] = (getnewadr() >> 8) & 0xff;
	sCmd[10+2] = getnewadr() & 0xff;
	write(fd,sCmd,80);
	
	
// 	char sCmd[83];
// 	sprintf(sCmd,"$*23456%s  ",entry_text);
// 	sCmd[2] = 3; // pkttype textmessage
// 	sCmd[3] = getadr();
// 	sCmd[4] = 1;
// 	sCmd[5] = getnewadr(); // this should be the address, hopefully
// 	sCmd[6] = (unsigned char)strlen(entry_text)+1;
// 	fixcmd();
// 	write(fd,sCmd,83);
	printf("set new id \n");	
}
int main (int argc, char *argv[]){
	
	GtkBuilder      *builder; 
	GtkWidget       *window;
	openfile();
	gtk_init (&argc, &argv);
 	initserial();
	builder = gtk_builder_new ();
	gtk_builder_add_from_file (builder, "mainwin.xml", NULL);
	window = GTK_WIDGET (gtk_builder_get_object (builder, "window"));
	entry1 = GTK_WIDGET(gtk_builder_get_object(builder,"entry1"));
	entry2 = GTK_WIDGET(gtk_builder_get_object(builder,"entry2"));
	entry3 = GTK_WIDGET(gtk_builder_get_object(builder,"entry3"));
	entry4 = GTK_WIDGET(gtk_builder_get_object(builder,"entry4"));
	entry5 = GTK_WIDGET(gtk_builder_get_object(builder,"entry5"));
	entry6 = GTK_WIDGET(gtk_builder_get_object(builder,"entry6"));
	statusbar = GTK_WIDGET(gtk_builder_get_object(builder,"statusbar1"));		
	chkbtn = GTK_WIDGET(gtk_builder_get_object(builder,"checkbutton1"));
	gtk_builder_connect_signals (builder, NULL);
	
	
	g_object_unref (G_OBJECT (builder));
        
	gtk_widget_show (window);                
	gtk_main ();
 
	return 0;
}

int getadr(){
	tmptext = gtk_entry_get_text (GTK_ENTRY (entry2));
	return atoi(tmptext);
}
int getnewadr(){
	tmptext = gtk_entry_get_text (GTK_ENTRY (entry4));
	return atoi(tmptext);
}
int gettimes(){
	tmptext = gtk_entry_get_text (GTK_ENTRY (entry3));
	return atoi(tmptext);
}
int getinfrate(){
	tmptext = gtk_entry_get_text (GTK_ENTRY (entry5));
	return atoi(tmptext);
}
int getcurerate(){
	tmptext = gtk_entry_get_text (GTK_ENTRY (entry6));
	return atoi(tmptext);
}
void initserial(){
	
	fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd < 0)
	{
		perror("/dev/ttyUSB0");
		
	}
	
	struct termios options;
	// Get the current options for the port...
	tcgetattr(fd, &options);
	// Set the baud rates to 19200...
	cfsetispeed(&options, B9600);
	cfsetospeed(&options, B9600);
	// Enable the receiver and set local mode...
	options.c_cflag |= (CLOCAL | CREAD);

	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_oflag = 0;
	options.c_iflag = 0;
	options.c_iflag = IGNPAR | ICRNL;
	options.c_lflag = 0;
	options.c_cflag |= CS8;
	tcflush(fd,TCIFLUSH);

	// Set the new options for the port...
	tcsetattr(fd, TCSANOW, &options);
	
	
// // 	struct termios oldtio, newtio;       //place for old and new port settings for serial port
// 	struct termios options;
// 	// Get the current options for the port...
// 	tcgetattr(fd, &options);
// 	// Set the baud rates to 19200...
// 	cfsetispeed(&options, B9600);
// 	cfsetospeed(&options, B9600);
// 	// Enable the receiver and set local mode...
// 	options.c_cflag |= (CLOCAL | CREAD);
// 
// 	options.c_cflag &= ~PARENB;
// 	options.c_cflag &= ~CSTOPB;
// 	options.c_cflag &= ~CSIZE;
// 	options.c_cflag |= CS8;
// 
// 	// Set the new options for the port...
// 	tcsetattr(fd, TCSANOW, &options);
}
void fixcmd(){
	entry_text = gtk_entry_get_text (GTK_ENTRY (entry1));
	unsigned int pek = 0;
	int q = 0;
	while(entry_text[q] != 0){
		if((unsigned char)entry_text[q] == 195){
			if((unsigned char)entry_text[q+1] == 165){ // found 
				inptext[pek] = 134;
				pek++;
			}
			else if((unsigned char)entry_text[q+1] == 164){
				inptext[pek] = 132;
				pek++;
			}
			else if((unsigned char)entry_text[q+1] == 182){
				inptext[pek] = 148;
				pek++;
			}
			else if((unsigned char)entry_text[q+1] == 133){
				inptext[pek] = 143;
				pek++;
			}
			else if((unsigned char)entry_text[q+1] == 132){
				inptext[pek] = 142;
				pek++;
			}
			else if((unsigned char)entry_text[q+1] == 150){
				inptext[pek] = 153;
				pek++;
			}
			
			q++;
			q++;
		}
		else{
		inptext[pek] = entry_text[q];
		pek++;
		q++;
		}
	}
	int i;
	inptext[pek] = 32;
	inptext[pek+1] = 32;
	inptext[pek+2] = 32;
	inptext[pek+3] = 32;
	inptext[pek+4] = 0;;
	pek += 4;
// 	for(i = 0; i < pek; i++){
// 		printf("tset %d q= %d\n",inptext[i],pek);
// 	}
	
	inptextlength = pek;
	
// 	int textpekare = 0;
// 	for(q = 0; q < sCmd[6]; q++){
// 		if((unsigned char)entry_text[q] == 195){
// 			if((unsigned char)entry_text[q+1] == 165){ // found 
// 				sCmd[7+textpekare] = 3;
// 			}
// 			else if((unsigned char)entry_text[q+1] == 164){
// 				sCmd[7+textpekare] = 4;
// 			}
// 			else if((unsigned char)entry_text[q+1] == 182){
// 				sCmd[7+textpekare] = 5;
// 			}
// 			else if((unsigned char)entry_text[q+1] == 133){
// 				sCmd[7+textpekare] = 6;
// 			}
// 			else if((unsigned char)entry_text[q+1] == 132){
// 				sCmd[7+textpekare] = 7;
// 			}
// 			else if((unsigned char)entry_text[q+1] == 150){
// 				sCmd[7+textpekare] = 8;
// 			}
// 			
// 		q++;
// 		}
// 		else{
// 			sCmd[7+textpekare] = entry_text[q];
// 		}
// 		textpekare++;	
// 	}
// 	sCmd[6] = textpekare;
}

void openfile(){
#
	file = fopen ( filename, "a+" );
#
}
void closefile(){
	fclose ( file );
}

int getid(){
	readlast();
	char nbr[5];
	int nbrof = 0;
	int i;
	for(i = 0; i < 3; i++){
		if(nbr[i] != ' '){
			nbrof++;
			nbr[i] = line[i];	
		}
		else
			break;
	}
	return atoi(nbr);
	
}
void writenewnolla(){
	char nolla[128];
	entry_text = gtk_entry_get_text (GTK_ENTRY (entry1));
	sprintf(nolla,"%i %s\n",getid()+1,entry_text);
	printf("\n%s",nolla);
	fwrite(nolla, sizeof(char), strlen(nolla), file);
	readlast();
}
void readlast(){
	char temp[60];
	if(file != NULL)
		closefile();
	openfile();
	if ( file != NULL )
#
	{
		while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
#
		{
#
		}
		
		gtk_statusbar_push(GTK_STATUSBAR(statusbar),1,line); 
		printf("\n%s",line);
		
#
	}
}
		
