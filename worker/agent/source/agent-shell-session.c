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
#include <error-code.h>
#include <agent-server.h>



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


static void
character_remover(gchar** string, gchar* character)
{
    char **split = g_strsplit(*string, character, -1);
    memset(*string,0,strlen(*string));
    *string = g_strjoinv("", split);
    g_strfreev(split);
}

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

void
initialize_shell_session(AgentServer* agent,
                         SoupMessage* message)
{
    GError* error = NULL;
    ShellSession session;
    session.message = message;

    GRand* random = g_rand_new();
    gchar* random_int = g_rand_int(random);

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
    if(!session.input_file || 
       !session.output_file)
    {
        return;
    }

    g_file_set_contents(input_file__path_char,
        message->request_body->data,
        message->request_body->length,&error);
    if(error){return;}


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
    {
        soup_message_set_response(session.message,
            "application/json",SOUP_MEMORY_COPY,buffer,strlen(buffer));
    }


    


    free(random_string);
    g_file_delete(session.input_file,NULL,NULL);
    g_file_delete(session.output_file,NULL,NULL);
    clean_childprocess(session.process); 
}


