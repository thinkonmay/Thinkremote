/**
 * @file agent-shell-session.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <agent-shell-session.h>
#include <agent-type.h>
#include <agent-child-process.h>
#include <agent-socket.h>


#include <logging.h>
#include <message-form.h>
#include <agent-server.h>

#include <glib.h>




#ifdef G_OS_WIN32
#include <Windows.h>

#endif
   
struct _ShellSession
{
    ChildProcess* process;

    GFile* input_file;

    GFile* output_file;

    SoupMessage* message;
};



void  state_handle(ChildProcess* proc,
                AgentServer* agent,
                gpointer data)
{

}

void  output_handle(GBytes* buffer,
                    AgentServer* agent,
                    gpointer data)
{

}


#ifndef G_OS_WIN32
void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}  

void itoa(int n, char s[],int ten)
{
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}  
#endif

gboolean
initialize_shell_session_from_byte(AgentServer* agent,
                                    GBytes* input,
                                    gchar* output)
{
    GError* error = NULL;
    ShellSession session;

    GRand* random = g_rand_new();
    guint32 random_int = g_rand_int(random);

    gchar random_string[100] = {0};
    itoa(random_int,random_string,10);

    GString * input_file_path = g_string_new(g_get_current_dir());
    GString * output_file_path = g_string_new(g_get_current_dir());
    g_string_append(input_file_path,"\\");
    g_string_append(output_file_path,"\\");
    g_string_append(input_file_path,random_string);
    g_string_append(output_file_path,random_string);
    g_string_append(input_file_path,".ps1");
    g_string_append(output_file_path,".txt");

    gchar* input_file__path_char = g_string_free(input_file_path,FALSE);
    gchar* output_file__path_char = g_string_free(output_file_path,FALSE);

    session.input_file = g_file_new_for_path(input_file__path_char);
    session.output_file = g_file_new_for_path(output_file__path_char);


    if(!session.input_file || !session.output_file)
        return FALSE;

    g_file_set_contents(input_file__path_char,
        g_bytes_get_data(input,NULL),
        g_bytes_get_size(input),&error);

    if(error)
        return;


    GString* string = g_string_new("powershell ");
    g_string_append(string,input_file__path_char);
    g_string_append(string," | out-file -encoding ASCII ");
    g_string_append(string,output_file__path_char);
    gchar* script = g_string_free(string,FALSE); 


    session.process = create_new_child_process(script,
                                                (ChildStdErrHandle)output_handle,
                                                (ChildStdOutHandle)output_handle,
                                                (ChildStateHandle)state_handle,
                                                agent,&session);
    if(!session.process)
        return FALSE;

    
    wait_for_childproces(session.process);

    gchar* buffer;
    gsize file_size;
    g_file_get_contents(output_file__path_char,&buffer,&file_size,&error);

    if(file_size < 8192) 
        memcpy(output,buffer,file_size);
    else 
        memcpy(output,buffer,8191);

    g_file_delete(session.input_file,NULL,NULL);
    g_file_delete(session.output_file,NULL,NULL);
    clean_childprocess(session.process); 
    return TRUE;
}



void
initialize_shell_session(AgentServer* agent,
                         SoupMessage* message)
{
    GError* error = NULL;
    ShellSession session;
    session.message = message;

    GRand* random = g_rand_new();
    guint32 random_int = g_rand_int(random);

    gchar* random_string = malloc(100);
    memset(random_string,0,100);

    itoa(random_int,random_string,10);

    GString * input_file_path = g_string_new(g_get_current_dir());
    GString * output_file_path = g_string_new(g_get_current_dir());
    g_string_append(input_file_path,"\\");
    g_string_append(output_file_path,"\\");
    g_string_append(input_file_path,random_string);
    g_string_append(output_file_path,random_string);
    g_string_append(input_file_path,".ps1");
    g_string_append(output_file_path,".txt");

    gchar* input_file__path_char = g_string_free(input_file_path,FALSE);
    gchar* output_file__path_char = g_string_free(output_file_path,FALSE);

    session.input_file = g_file_new_for_path(input_file__path_char);
    session.output_file = g_file_new_for_path(output_file__path_char);

    message->status_code = SOUP_STATUS_OK;

    if(!session.input_file || !session.output_file)
        return;

    g_file_set_contents(input_file__path_char,
        message->request_body->data,
        message->request_body->length,&error);

    if(error)
        return;


    GString* string = g_string_new("powershell ");
    g_string_append(string,input_file__path_char);
    g_string_append(string," | out-file -encoding ASCII ");
    g_string_append(string,output_file__path_char);
    gchar* script = g_string_free(string,FALSE); 


    session.process = create_new_child_process(script,
                                                (ChildStdErrHandle)output_handle,
                                                (ChildStdOutHandle)output_handle,
                                                (ChildStateHandle)state_handle,
                                                agent,&session);
    if(!session.process)
        return;

    
    wait_for_childproces(session.process);

    gchar* buffer;
    gsize file_size;
    g_file_get_contents(output_file__path_char,&buffer,&file_size,&error);

    if(buffer)
        soup_message_set_response(session.message, "application/json",SOUP_MEMORY_COPY,buffer,strlen(buffer));

    free(random_string);
    g_file_delete(session.input_file,NULL,NULL);
    g_file_delete(session.output_file,NULL,NULL);
    clean_childprocess(session.process); 
}


