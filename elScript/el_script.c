#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <map>
#include <vector>
#include <time.h>

#define INT 0
#define INTSIZE sizeof( int )
#define CHAR 1
#define CHARSIZE sizeof( char )
#define STRING 2
#define LOOPVAR 3

struct var
{
        char type;
        void *data;
};

int runscript( const char *s )
{
        FILE *f = fopen( s, "r" );

        if( f == 0 )
        {
                printf( "Can't open %s\n", s );
                return -1;
        }

        std::map<char *, struct var *> *vars = new std::map<char *, struct var *>();

        char line[256];

        memset( line, '\0', 256 );

        int line_count = 0;

        while( !feof( f ) )
        {
                fgets( line, 256, f );
                line_count++;

                if( strstr( line, "VARS" ) )
                {
                        while( !feof( f ) )
                        {
                                fgets( line, 256, f );
                                line_count++;


                                if( line[0] == '\n' || line[0] == '\r' )
                                        continue;
                                else
                                        if( strstr( line, "EVARS" ) )
                                                break;
                                        else
                                        {
                                                char *t = strtok( line, " " );

                                                if( strstr( t, "int" ) )
                                                {
                                                        t = strtok( 0, " =" );

                                                        char *n = (char *)malloc( strlen( t ) + 1 );

                                                        strcpy( n, t );

                                                        struct var *v = (struct var *)malloc( sizeof( struct var ) );

                                                        v->type = INT;
                                                        v->data = malloc( INTSIZE );

                                                        t = strtok( 0, " =\r\n" );

                                                        *(int *)v->data = atoi( t );

                                                        vars->insert( vars->begin(), std::pair<char *, struct var *>( n, v ) );
                                                }
                                                else if( strstr( t, "string" ) )
                                                {
                                                        t = strtok( 0, " \"" );

                                                        char *n = (char *)malloc( strlen( t ) + 1 );

                                                        strcpy( n, t );

                                                        struct var *v = (struct var *)malloc( sizeof( struct var ) );

                                                        t = strtok( 0, "=\r\n" );

                                                        t++;
                                                        t++;

                                                        t[strlen( t ) - 1] = '\0';

                                                        v->type = STRING;

                                                        v->data = malloc( strlen( t ) + 1 );

                                                        strcpy( (char *)v->data, t );

                                                        vars->insert( vars->begin(), std::pair<char *, struct var *>( n, v ) );
                                                }
                                                else if( strstr( t, "char" ) )
                                                {
                                                        t = strtok( 0, " =" );

                                                        char *n = (char *)malloc( strlen( t ) + 1 );

                                                        strcpy( n, t );

                                                        struct var *v = (struct var *)malloc( sizeof( struct var ) );

                                                        t = strtok( 0, " =\'\"\r\n" );

                                                        v->type = CHAR;

                                                        v->data = malloc( CHARSIZE );

                                                        *(char *)v->data = t[0];

                                                        vars->insert( vars->begin(), std::pair<char *, struct var *>( n, v ) );
                                                }
                                        }
                                }
                }
                else if( strstr( line, "CODE" ) )
                {
                        bool looping = false;
                        int loopenter = 0;
                        int loopcount = 0;
                        int loopfilepos = 0;
                        char loopname[32];
                        int inIf = 0;
                        bool skipIf = false;
                        bool ecode = false;
                        while( !feof( f ) && !ecode )
                        {
                                switch( loopenter )
                                {
                                        case 1:
                                                while( !feof( f ) && loopcount >= 0 )
                                                {
                                                        case 0:
                                                        if( fgets( line, 256, f ) == 0 )
                                                        {
                                                                delete vars;
                                                                fclose( f );
                                                                return 0;
                                                        }

                                                        line_count++;

                                                        if( skipIf && strstr( line, "ELSE" ) )
                                                        {
                                                                skipIf = false;
                                                                break;
                                                        }

                                                        if( !strstr( line, "EIF" ) && inIf > 0 && skipIf )
                                                                break;

                                                        if( strstr( line, "ECODE" ) )
                                                        {
                                                                ecode = true;
                                                                break;
                                                        }
                                                        else if( line[0] == '\n' || line[0] == '\r' )
                                                                continue;
                                                        else if( strstr( line, "ELOOP" ) && looping )
                                                        {
                                                                loopcount--;
                                                                if( strcmp( loopname, "!@#$" ) != 0)
                                                                    for( std::map<char *,struct var *>::iterator it = vars->begin(); it != vars->end(); it++ )
                                                                    {
                                                                            if( (*it).second->type == INT && strcmp( (*it).first, loopname ) == 0 )
                                                                            {
                                                                                    *(int *)(*it).second->data = loopcount;
                                                                                    break;
                                                                            }
                                                                    }

                                                                if( loopcount <= 0 )
                                                                {
                                                                        loopenter = 0;
                                                                        looping = false;
                                                                }
                                                                else
                                                                {
                                                                        fseek( f, loopfilepos, SEEK_SET );
                                                                }

                                                                break;
                                                        }
                                                        else if( strstr( line, "LOOP" ) && !looping )
                                                        {
                                                                loopenter = 1;
                                                                looping = true;
                                                                loopfilepos = ftell( f );

                                                                char *a = strtok( line, ":\r\n" );
                                                                a = strtok( 0, "\r\n" );

                                                                if( a[0] != '$' )
                                                                {
                                                                    for( std::map<char *,struct var *>::iterator it = vars->begin(); it != vars->end(); it++ )
                                                                    {
                                                                            if( (*it).second->type == INT && strcmp( (*it).first, a ) == 0 )
                                                                            {
                                                                                    loopcount = *(int *)(*it).second->data;
                                                                                    strcpy( loopname, a );
                                                                                    break;
                                                                            }
                                                                            else
                                                                            {
                                                                                    loopcount = 0;
                                                                            }
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                    loopcount = atoi( a + 1 );
                                                                    strcpy( loopname, "!@#$");
                                                                }

                                                                if( loopcount < 0 )
                                                                {
                                                                        looping = false;
                                                                        loopenter = 0;
                                                                        printf( "Error Line %d, Varible \"%s\" Not Found\n", line_count, a );
                                                                        delete vars;
                                                                        fclose( f );
                                                                        return 0;
                                                                }
                                                        }
                                                        else
                                                        {
                                                                char *t = strtok( line, " ,\r\n" );

                                                                if( strstr( t, "ADD" ) )
                                                                {
                                                                        char *a = strtok( 0, " ,\r\n" );
                                                                        char *b = strtok( 0, " ,\r\n" );

                                                                        std::map<char *, struct var *>::iterator i = vars->end(), j = vars->end();

                                                                        for( std::map<char *, struct var *>::iterator it = vars->begin(); it != vars->end(); it++ )
                                                                        {
                                                                                if( strcmp( (*it).first, a ) == 0 )
                                                                                        i = it;
                                                                                else if( b[0] != '$' && strcmp( (*it).first, b ) == 0 )
                                                                                        j = it;
                                                                        }

                                                                        if( i == vars->end() )
                                                                        {
                                                                                printf( "Error Line: %d, Varible \"%s\" Not Found\n",line_count, a );
                                                                                delete vars;
                                                                                fclose( f );
                                                                                return 0;
                                                                        }
                                                                        else if( b[0] != '$' && j == vars->end() )
                                                                        {
                                                                                printf( "Error Line: %d, Varible \"%s\" Not Found\n",line_count, b );
                                                                                delete vars;
                                                                                fclose( f );
                                                                                return 0;
                                                                        }

                                                                        if( b[0] != '$' )
                                                                                if( (*i).second->type == INT )
                                                                                        *(int *)(*i).second->data = *(int *)(*i).second->data + *(int *)(*j).second->data;
                                                                                else
                                                                                        *(char *)(*i).second->data = *(char *)(*i).second->data + *(char *)(*j).second->data;
                                                                        else
                                                                                if( (*i).second->type == INT )
                                                                                        *(int *)(*i).second->data = *(int *)(*i).second->data + atoi( b + 1 );
                                                                                else
                                                                                        *(char *)(*i).second->data = *(char *)(*i).second->data + atoi( b + 1 );
                                                                }
                                                                else if( strstr( t, "SUB" ) )
                                                                {
                                                                        char *a = strtok( 0, " ,\r\n" );
                                                                        char *b = strtok( 0, " ,\r\n" );

                                                                        std::map<char *, struct var *>::iterator i = vars->end(), j = vars->end();

                                                                        for( std::map<char *, struct var *>::iterator it = vars->begin(); it != vars->end(); it++ )
                                                                        {
                                                                                if( strcmp( (*it).first, a ) == 0 )
                                                                                        i = it;
                                                                                else if( b[0] != '$' && strcmp( (*it).first, b ) == 0 )
                                                                                        j = it;
                                                                        }

                                                                        if( i == vars->end() )
                                                                        {
                                                                                printf( "Error Line: %d, Varible \"%s\" Not Found\n",line_count, a );
                                                                                delete vars;
                                                                                fclose( f );
                                                                                return 0;
                                                                        }
                                                                        else if( b[0] != '$' && j == vars->end() )
                                                                        {
                                                                                printf( "Error Line: %d, Varible \"%s\" Not Found\n",line_count, b );
                                                                                delete vars;
                                                                                fclose( f );
                                                                                return 0;
                                                                        }

                                                                        if( b[0] != '$' )
                                                                                if( (*i).second->type == INT )
                                                                                        *(int *)(*i).second->data = *(int *)(*i).second->data - *(int *)(*j).second->data;
                                                                                else
                                                                                        *(char *)(*i).second->data = *(char *)(*i).second->data - *(char *)(*j).second->data;
                                                                        else
                                                                                if( (*i).second->type == INT )
                                                                                        *(int *)(*i).second->data = *(int *)(*i).second->data - atoi( b + 1 );
                                                                                else
                                                                                        *(char *)(*i).second->data = *(char *)(*i).second->data - atoi( b + 1 );
                                                                }
                                                                else if( strstr( t, "MUL" ) )
                                                                {
                                                                        char *a = strtok( 0, " ,\r\n" );
                                                                        char *b = strtok( 0, " ,\r\n" );

                                                                        std::map<char *, struct var *>::iterator i = vars->end(), j = vars->end();

                                                                        for( std::map<char *, struct var *>::iterator it = vars->begin(); it != vars->end(); it++ )
                                                                        {
                                                                                if( strcmp( (*it).first, a ) == 0 )
                                                                                        i = it;
                                                                                else if( b[0] != '$' && strcmp( (*it).first, b ) == 0 )
                                                                                        j = it;
                                                                        }

                                                                        if( i == vars->end() )
                                                                        {
                                                                                printf( "Error Line: %d, Varible \"%s\" Not Found\n",line_count, a );
                                                                                delete vars;
                                                                                fclose( f );
                                                                                return 0;
                                                                        }
                                                                        else if( b[0] != '$' && j == vars->end() )
                                                                        {
                                                                                printf( "Error Line: %d, Varible \"%s\" Not Found\n",line_count, b );
                                                                                delete vars;
                                                                                fclose( f );
                                                                                return 0;
                                                                        }

                                                                        if( b[0] != '$' )
                                                                                if( (*i).second->type == INT )
                                                                                        *(int *)(*i).second->data = *(int *)(*i).second->data + *(int *)(*j).second->data;
                                                                                else
                                                                                        *(char *)(*i).second->data = *(char *)(*i).second->data + *(char *)(*j).second->data;
                                                                        else
                                                                                if( (*i).second->type == INT )
                                                                                        *(int *)(*i).second->data = *(int *)(*i).second->data + atoi( b + 1 );
                                                                                else
                                                                                        *(char *)(*i).second->data = *(char *)(*i).second->data + atoi( b + 1 );
                                                                }
                                                                else if( strstr( t, "DIV" ) )
                                                                {
                                                                        char *a = strtok( 0, " ,\r\n" );
                                                                        char *b = strtok( 0, " ,\r\n" );

                                                                        std::map<char *, struct var *>::iterator i = vars->end(), j = vars->end();

                                                                        for( std::map<char *, struct var *>::iterator it = vars->begin(); it != vars->end(); it++ )
                                                                        {
                                                                                if( strcmp( (*it).first, a ) == 0 )
                                                                                        i = it;
                                                                                else if( b[0] != '$' && strcmp( (*it).first, b ) == 0 )
                                                                                        j = it;
                                                                        }

                                                                        if( i == vars->end() )
                                                                        {
                                                                                printf( "Error Line: %d, Varible \"%s\" Not Found\n",line_count, a );
                                                                                delete vars;
                                                                                fclose( f );
                                                                                return 0;
                                                                        }
                                                                        else if( b[0] != '$' && j == vars->end() )
                                                                        {
                                                                                printf( "Error Line: %d, Varible \"%s\" Not Found\n",line_count, b );
                                                                                delete vars;
                                                                                fclose( f );
                                                                                return 0;
                                                                        }

                                                                        if( b[0] != '$' )
                                                                                if( (*i).second->type == INT )
                                                                                        *(int *)(*i).second->data = *(int *)(*i).second->data + *(int *)(*j).second->data;
                                                                                else
                                                                                        *(char *)(*i).second->data = *(char *)(*i).second->data + *(char *)(*j).second->data;
                                                                        else
                                                                                if( (*i).second->type == INT )
                                                                                        *(int *)(*i).second->data = *(int *)(*i).second->data + atoi( b + 1 );
                                                                                else
                                                                                        *(char *)(*i).second->data = *(char *)(*i).second->data + atoi( b + 1 );
                                                                }
                                                                else if( strstr( t, "MOV" ) )
                                                                {
                                                                        char *a = strtok( 0, " ,\r\n" );
                                                                        char *b = strtok( 0, " ,\r\n" );

                                                                        std::map<char *, struct var *>::iterator i = vars->end(), j = vars->end();

                                                                        for( std::map<char *, struct var *>::iterator it = vars->begin(); it != vars->end(); it++ )
                                                                        {
                                                                                if( strcmp( (*it).first, a ) == 0 )
                                                                                        i = it;
                                                                                else if( ( b[0] != '$' || b[0] != '\'' )  && strcmp( (*it).first, b ) == 0 )
                                                                                        j = it;
                                                                        }

                                                                        if( i == vars->end() )
                                                                        {
                                                                                printf( "Error Line: %d, Varible \"%s\" Not Found\n",line_count, a );
                                                                                delete vars;
                                                                                fclose( f );
                                                                                return 0;
                                                                        }
                                                                        else if( b[0] != '$' && j == vars->end() )
                                                                        {
                                                                                printf( "Error Line: %d, Varible \"%s\" Not Found\n",line_count, b );
                                                                                delete vars;
                                                                                fclose( f );
                                                                                return 0;
                                                                        }

                                                                        if( b[0] != '$' && b[0] != '\'' )
                                                                                if( (*i).second->type == INT )
                                                                                        *(int *)(*i).second->data = *(int *)(*i).second->data = *(int *)(*j).second->data;
                                                                                else
                                                                                        *(char *)(*i).second->data = *(char *)(*i).second->data = *(char *)(*j).second->data;
                                                                        else
                                                                                if( b[0] == '$' )
                                                                                        *(int *)(*i).second->data = *(int *)(*i).second->data = atoi( b + 1 );
                                                                                else if( b[0] == '\'' )
                                                                                        *(char *)(*i).second->data = *(char *)(*i).second->data = b[1];
                                                                }
                                                                else if( strstr( t, "MOD" ) )
                                                                {
                                                                        char *a = strtok( 0, " ,\r\n" );
                                                                        char *b = strtok( 0, " ,\r\n" );

                                                                        std::map<char *, struct var *>::iterator i = vars->end(), j = vars->end();

                                                                        for( std::map<char *, struct var *>::iterator it = vars->begin(); it != vars->end(); it++ )
                                                                        {
                                                                                if( strcmp( (*it).first, a ) == 0 )
                                                                                        i = it;
                                                                                else if( b[0] != '$' && strcmp( (*it).first, b ) == 0 )
                                                                                        j = it;
                                                                        }

                                                                        if( i == vars->end() )
                                                                        {
                                                                                printf( "Error Line: %d, Varible \"%s\" Not Found\n",line_count, a );
                                                                                delete vars;
                                                                                fclose( f );
                                                                                return 0;
                                                                        }
                                                                        else if( b[0] != '$' && j == vars->end() )
                                                                        {
                                                                                printf( "Error Line: %d, Varible \"%s\" Not Found\n",line_count, b );
                                                                                delete vars;
                                                                                fclose( f );
                                                                                return 0;
                                                                        }

                                                                        if( b[0] != '$' )
                                                                                if( (*i).second->type == INT )
                                                                                        *(int *)(*i).second->data = *(int *)(*i).second->data % *(int *)(*j).second->data;
                                                                                else
                                                                                        *(char *)(*i).second->data = *(char *)(*i).second->data % *(char *)(*j).second->data;
                                                                        else
                                                                                if( (*i).second->type == INT )
                                                                                        *(int *)(*i).second->data = *(int *)(*i).second->data % atoi( b + 1 );
                                                                                else
                                                                                        *(char *)(*i).second->data = *(char *)(*i).second->data % atoi( b + 1 );
                                                                }
                                                                else if( strstr( t, "EIF" ) )
                                                                {
                                                                        inIf--;
                                                                        skipIf = false;

                                                                        if( inIf < 0 )
                                                                                inIf = 0;
                                                                        break;
                                                                }
                                                                else if( strstr( t, "ELSE" ) )
                                                                {
                                                                        if( skipIf == false )
                                                                                skipIf = true;
                                                                        else
                                                                                skipIf = false;
                                                                        break;
                                                                }
                                                                else if( strstr( t, "IFEQ" ) )
                                                                {
                                                                        inIf++;
                                                                        char *a = strtok( 0, " ,\r\n" );
                                                                        char *b = strtok( 0, " ,\r\n" );

                                                                        std::map<char *, struct var *>::iterator i = vars->end(), j = vars->end();

                                                                        for( std::map<char *, struct var *>::iterator it = vars->begin(); it != vars->end(); it++ )
                                                                        {
                                                                                if( strcmp( (*it).first, a ) == 0 )
                                                                                        i = it;
                                                                                else if( b[0] != '$' && strcmp( (*it).first, b ) == 0 )
                                                                                        j = it;
                                                                        }

                                                                        if( i == vars->end() )
                                                                        {
                                                                                printf( "Error Line: %d, Varible \"%s\" Not Found\n",line_count, a );
                                                                                delete vars;
                                                                                fclose( f );
                                                                                return 0;
                                                                        }
                                                                        else if( b[0] != '$' && j == vars->end() )
                                                                        {
                                                                                printf( "Error Line: %d, Varible \"%s\" Not Found\n",line_count, b );
                                                                                delete vars;
                                                                                fclose( f );
                                                                                return 0;
                                                                        }

                                                                        if( b[0] != '$' )
                                                                                if( (*i).second->type == INT )
                                                                                        skipIf = *(int *)(*i).second->data != *(int *)(*j).second->data;
                                                                                else
                                                                                        skipIf = *(char *)(*i).second->data != *(char *)(*j).second->data;
                                                                        else
                                                                                if( (*i).second->type == INT )
                                                                                        skipIf = *(int *)(*i).second->data != atoi( b + 1 );
                                                                                else
                                                                                        skipIf = *(char *)(*i).second->data != atoi( b + 1 );
                                                                }
                                                                else if( strstr( t, "IFNEQ" ) )
                                                                {
                                                                        inIf++;

                                                                        char *a = strtok( 0, " ,\r\n" );
                                                                        char *b = strtok( 0, " ,\r\n" );

                                                                        std::map<char *, struct var *>::iterator i = vars->end(), j = vars->end();

                                                                        for( std::map<char *, struct var *>::iterator it = vars->begin(); it != vars->end(); it++ )
                                                                        {
                                                                                if( strcmp( (*it).first, a ) == 0 )
                                                                                        i = it;
                                                                                else if( b[0] != '$' && strcmp( (*it).first, b ) == 0 )
                                                                                        j = it;
                                                                        }

                                                                        if( i == vars->end() )
                                                                        {
                                                                                printf( "Error Line: %d, Varible \"%s\" Not Found\n",line_count, a );
                                                                                delete vars;
                                                                                fclose( f );
                                                                                return 0;
                                                                        }
                                                                        else if( b[0] != '$' && j == vars->end() )
                                                                        {
                                                                                printf( "Error Line: %d, Varible \"%s\" Not Found\n",line_count, b );
                                                                                delete vars;
                                                                                fclose( f );
                                                                                return 0;
                                                                        }

                                                                        if( b[0] != '$' )
                                                                                if( (*i).second->type == INT )
                                                                                        skipIf = *(int *)(*i).second->data == *(int *)(*j).second->data;
                                                                                else
                                                                                        skipIf = *(char *)(*i).second->data == *(char *)(*j).second->data;
                                                                        else
                                                                                if( (*i).second->type == INT )
                                                                                        skipIf = *(int *)(*i).second->data == atoi( b + 1 );
                                                                                else
                                                                                        skipIf = *(char *)(*i).second->data == atoi( b + 1 );
                                                                }
                                                                else if( strstr( t, "IFG" ) )
                                                                {
                                                                        inIf++;

                                                                        char *a = strtok( 0, " ,\r\n" );
                                                                        char *b = strtok( 0, " ,\r\n" );

                                                                        std::map<char *, struct var *>::iterator i = vars->end(), j = vars->end();

                                                                        for( std::map<char *, struct var *>::iterator it = vars->begin(); it != vars->end(); it++ )
                                                                        {
                                                                                if( strcmp( (*it).first, a ) == 0 )
                                                                                        i = it;
                                                                                else if( b[0] != '$' && strcmp( (*it).first, b ) == 0 )
                                                                                        j = it;
                                                                        }

                                                                        if( i == vars->end() )
                                                                        {
                                                                                printf( "Error Line: %d, Varible \"%s\" Not Found\n",line_count, a );
                                                                                delete vars;
                                                                                fclose( f );
                                                                                return 0;
                                                                        }
                                                                        else if( b[0] != '$' && j == vars->end() )
                                                                        {
                                                                                printf( "Error Line: %d, Varible \"%s\" Not Found\n",line_count, b );
                                                                                delete vars;
                                                                                fclose( f );
                                                                                return 0;
                                                                        }

                                                                        if( b[0] != '$' )
                                                                                if( (*i).second->type == INT )
                                                                                        skipIf = *(int *)(*i).second->data < *(int *)(*j).second->data;
                                                                                else
                                                                                        skipIf = *(char *)(*i).second->data < *(char *)(*j).second->data;
                                                                        else
                                                                                if( (*i).second->type == INT )
                                                                                        skipIf = *(int *)(*i).second->data < atoi( b + 1 );
                                                                                else
                                                                                        skipIf = *(char *)(*i).second->data < atoi( b + 1 );
                                                                }
                                                                else if( strstr( t, "IFL" ) )
                                                                {
                                                                        inIf++;

                                                                        char *a = strtok( 0, " ,\r\n" );
                                                                        char *b = strtok( 0, " ,\r\n" );

                                                                        std::map<char *, struct var *>::iterator i = vars->end(), j = vars->end();

                                                                        for( std::map<char *, struct var *>::iterator it = vars->begin(); it != vars->end(); it++ )
                                                                        {
                                                                                if( strcmp( (*it).first, a ) == 0 )
                                                                                        i = it;
                                                                                else if( b[0] != '$' && strcmp( (*it).first, b ) == 0 )
                                                                                        j = it;
                                                                        }

                                                                        if( i == vars->end() )
                                                                        {
                                                                                printf( "Error Line: %d, Varible \"%s\" Not Found\n",line_count, a );
                                                                                delete vars;
                                                                                fclose( f );
                                                                                return 0;
                                                                        }
                                                                        else if( b[0] != '$' && j == vars->end() )
                                                                        {
                                                                                printf( "Error Line: %d, Varible \"%s\" Not Found\n",line_count, b );
                                                                                delete vars;
                                                                                fclose( f );
                                                                                return 0;
                                                                        }

                                                                        if( b[0] != '$' )
                                                                                if( (*i).second->type == INT )
                                                                                        skipIf = *(int *)(*i).second->data > *(int *)(*j).second->data;
                                                                                else
                                                                                        skipIf = *(char *)(*i).second->data > *(char *)(*j).second->data;
                                                                        else
                                                                                if( (*i).second->type == INT )
                                                                                        skipIf = *(int *)(*i).second->data > atoi( b + 1 );
                                                                                else
                                                                                        skipIf = *(char *)(*i).second->data > atoi( b + 1 );
                                                                }
                                                                else if( strstr( t, "PRINT" ) )
                                                                {
                                                                                char *a = strtok( 0, "\r\n" );

                                                                                std::map<char *, struct var *>::iterator i = vars->end();

                                                                                if( a[0] != '$' && a[0] != '\"' )
                                                                                {
                                                                                        for( std::map<char *, struct var *>::iterator it = vars->begin(); it != vars->end(); it++ )
                                                                                        {
                                                                                                if( strcmp( (*it).first, a ) == 0 )
                                                                                                        i = it;
                                                                                        }

                                                                                        if( i == vars->end() )
                                                                                        {
                                                                                                printf( "Error Line: %d, Varible \"%s\" Not Found\n",line_count, a );
                                                                                                delete vars;
                                                                                                fclose( f );
                                                                                                return 0;
                                                                                        }

                                                                                        switch( (*i).second->type )
                                                                                        {
                                                                                                case INT:
                                                                                                        printf( "%d\n", *(int *)(*i).second->data );
                                                                                                        break;

                                                                                                case CHAR:
                                                                                                        printf( "%c\n", *(char *)(*i).second->data );
                                                                                                        break;

                                                                                                case STRING:
                                                                                                        printf( "%s\n", (char *)(*i).second->data );
                                                                                                        break;
                                                                                        }
                                                                                }
                                                                                else
                                                                                {
                                                                                        switch( a[0] )
                                                                                        {
                                                                                                case '$':
                                                                                                        printf( "%s\n", (char *)( a + 1 ) );
                                                                                                        break;
                                                                                                case '\"':
                                                                                                        a++;
                                                                                                        a[strlen( a ) - 1] = '\0';
                                                                                                        printf( "%s\n", a );
                                                                                                        break;
                                                                                        }
                                                                                }
                                                                        }
                                                                else if( !looping )
                                                                        break;
                                                        }//else
                                                }//inner while loop
                                }//switch
                        }//main while loop
                }
        }

        delete vars;
        fclose( f );

        return 0;
}

int main( int argc, char **argv )
{
        int start = time( 0 );
        runscript( "script.txt" );
        printf( "Script finished in %d secs\n", (int)(time( 0 ) - start) );
        return 0;
}
