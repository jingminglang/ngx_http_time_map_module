
/*
 * Copyright (C) Jimila
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>



typedef struct {
    u_char                              *opt;
    ngx_http_variable_value_t            *value;
    int                                     time[6];
    int                                     b_time[6];
    int                                     e_time[6];
} ngx_http_time_node_t;


typedef struct {
    ngx_array_t                        *time_node;
    ngx_http_variable_value_t          *default_value;
    ngx_pool_t                         *pool;
} ngx_http_time_map_ctx_t;




static char *ngx_http_time_map_block(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_http_time_map(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_time_map_variable(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data);
static int gt(int a,int b);
static int lt(int a,int b);
static int cmp(int a[],int b[],int (*fun)(int,int),int index,int max);


static ngx_command_t  ngx_http_time_map_commands[] = {
    { ngx_string("time_map"),
      NGX_HTTP_MAIN_CONF|NGX_CONF_BLOCK|NGX_CONF_TAKE1,
      ngx_http_time_map_block,
      NGX_HTTP_MAIN_CONF_OFFSET,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_time_map_module_ctx = { NULL, NULL, NULL, NULL, NULL, NULL,NULL, NULL };




ngx_module_t  ngx_http_time_map_module = {
    NGX_MODULE_V1,
    &ngx_http_time_map_module_ctx,              /* module context */
    ngx_http_time_map_commands,                 /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};




static char *
ngx_http_time_map_block(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    char                              *rv;
    ngx_str_t                *value, name;
    ngx_conf_t                 save;
    ngx_http_time_map_ctx_t            *ctx;
    ngx_http_variable_t  *var ;

    value = cf->args->elts;
    name = value[1];
    if (name.data[0] != '$') {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "invalid variable name \"%V\"", &name);
        return NGX_CONF_ERROR;
    }
    name.len--;
    name.data++;

    var = ngx_http_add_variable(cf, &name, NGX_HTTP_VAR_CHANGEABLE);
    if (var == NULL) {
        return NGX_CONF_ERROR;
    }

    ctx = ngx_palloc(cf->pool, sizeof(ngx_http_time_map_ctx_t));
    if (ctx == NULL) {
        return NGX_CONF_ERROR;
    }
    ctx->pool = cf->pool;

    ctx->default_value = NULL;
    ctx->time_node = NULL;
    var->get_handler = ngx_http_time_map_variable;
    var->data = (uintptr_t) ctx;

    save = *cf;
    cf->ctx = ctx;

    cf->handler = ngx_http_time_map;
    cf->handler_conf = conf;

    rv = ngx_conf_parse(cf, NULL);
    *cf = save;

    if (ctx->default_value == NULL) {
        rv = NGX_CONF_ERROR;
    }
    return rv;

}


static ngx_int_t
ngx_http_time_map_variable(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_http_time_map_ctx_t      *tmcf;
    ngx_http_time_node_t *time_nodes;
    ngx_http_time_node_t *node;
    ngx_uint_t index=0;
    int year;
    int mounth;
    int day;
    int hour;
    int min;
    int sec;
    int now[6];
    time_t t;
    struct tm *p;
    t=time(NULL);
    p=localtime(&t);
    year=(1900+p->tm_year);
    mounth=(1+p->tm_mon);
    day=p->tm_mday;
    hour=p->tm_hour;
    min=p->tm_min;
    sec=p->tm_sec;
    now[0]=year;
    now[1]=mounth;
    now[2]=day;
    now[3]=hour;
    now[4]=min;
    now[5]=sec;

    tmcf = (ngx_http_time_map_ctx_t *) data;
    *v = *tmcf->default_value;


    time_nodes=(ngx_http_time_node_t *)tmcf->time_node->elts;
    for(;index < tmcf->time_node->nelts;index++) {
	    node = time_nodes+index;
	if(ngx_strcmp(node->opt, "gt")==0){
	   if(cmp(now,node->time,gt,0,6)){
             *v = *node->value;
	     break;
	   }
	}
	if(ngx_strcmp(node->opt, "lt")==0){
	   if(cmp(now,node->time,lt,0,6)){
             *v = *node->value;
	     break;
	   }
	}
	if(ngx_strcmp(node->opt, "at")==0){
	   if(cmp(now,node->b_time,gt,0,6) && cmp(now,node->e_time,lt,0,6)){
             *v = *node->value;
	     break;
	   }
	}
	if(ngx_strcmp(node->opt, "noat")==0){
	   if(cmp(now,node->b_time,lt,0,6) || cmp(now,node->e_time,gt,0,6)){
             *v = *node->value;
	     break;
	   }
	}
    }

    return NGX_OK;
}

static char *
ngx_http_time_map(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t                       *args;
    ngx_uint_t                      nelts;
    int count=0;
    ngx_http_time_map_ctx_t      *ctx;
    ngx_http_time_node_t    *time_node;

    ctx = cf->ctx;

    args = cf->args->elts;
    nelts = cf->args->nelts;
    
    if (ctx->time_node == NULL) {
        ctx->time_node = ngx_array_create(ctx->pool, 1, sizeof(ngx_http_time_node_t));
        if (ctx->time_node == NULL) {
            return NGX_CONF_ERROR;
        }
    }


    if (nelts == 2) {
	if (ngx_strcmp(args[0].data, "default") == 0) {
            if (ctx->default_value != NULL) {
                return NGX_CONF_ERROR;
            }

	    ctx->default_value = ngx_pcalloc(ctx->pool, sizeof(ngx_http_variable_t));
	    if (ctx->default_value == NULL) {
	        return NGX_CONF_ERROR;
            }

            ctx->default_value->len = args[1].len;
            ctx->default_value->data = args[1].data;

            ctx->default_value->not_found = 0;
            ctx->default_value->no_cacheable =0;
            ctx->default_value->valid =1;

	    return NGX_CONF_OK;
        }
    }
    if (nelts == 3) {
	time_node = (ngx_http_time_node_t *) ngx_array_push(ctx->time_node);
	time_node->opt = args[0].data;
	time_node->value = ngx_pcalloc(ctx->pool, sizeof(ngx_http_variable_t));
	if (time_node->value == NULL) {
	    return NGX_CONF_ERROR;
	}
	time_node->value->len = args[2].len;
	time_node->value->data = args[2].data;

	char *str;
	int b[6]={0},i=0;
	str = strtok((char*)args[1].data,"."); 
	while(str!=NULL) { 
	    if(i>=6) {
		ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "invalid time style");
		return NGX_CONF_ERROR;
	    } 
	    if(ngx_strcmp(str, "*")==0){
	      b[i]=-1;
	    } else {
	      b[i]=atoi(str);
	    }
	    i++;
	    str = strtok(NULL,"."); 
	} 

	time_node->time[0]=b[0];
	time_node->time[1]=b[1];
	time_node->time[2]=b[2];
	time_node->time[3]=b[3];
	time_node->time[4]=b[4];
	time_node->time[5]=b[5];

	return NGX_CONF_OK;
    }
    if (nelts == 4) {
	count++;
	time_node = (ngx_http_time_node_t *) ngx_array_push(ctx->time_node);
	time_node->opt = args[0].data;
	time_node->value = ngx_pcalloc(ctx->pool, sizeof(ngx_http_variable_t));
	if (time_node->value == NULL) {
	    return NGX_CONF_ERROR;
	}
	time_node->value->len = args[3].len;
	time_node->value->data = args[3].data;
	char *str;
	int b[6]={0},i=0;
	str = strtok((char*)args[1].data,"."); 
	while(str!=NULL) { 
	    if(i>=6) {
		ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "invalid time style");
		return NGX_CONF_ERROR;
	    } 
	    if(ngx_strcmp(str, "*")==0){
	      b[i]=-1;
	    } else {
	      b[i]=atoi(str);
	    }
	    i++;
	    str = strtok(NULL,"."); 
	} 

	time_node->b_time[0]=b[0];
	time_node->b_time[1]=b[1];
	time_node->b_time[2]=b[2];
	time_node->b_time[3]=b[3];
	time_node->b_time[4]=b[4];
	time_node->b_time[5]=b[5];
	i=0;
	str = strtok((char*)args[2].data,"."); 
	while(str!=NULL) { 
	    if(i>=6) {
		ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "invalid time style");
		return NGX_CONF_ERROR;
	    } 
	    if(ngx_strcmp(str, "*")==0){
	      b[i]=-1;
	    } else {
	      b[i]=atoi(str);
	    }
	    i++;
	    str = strtok(NULL,"."); 
	} 

	time_node->e_time[0]=b[0];
	time_node->e_time[1]=b[1];
	time_node->e_time[2]=b[2];
	time_node->e_time[3]=b[3];
	time_node->e_time[4]=b[4];
	time_node->e_time[5]=b[5];

	return NGX_CONF_OK;
    }
    return NGX_CONF_OK;
}


static int gt(int a,int b)
{
    if(a==b || b==-1) {
	return 2;
    }
    if(a>b) {
	return 1;
    }
    if(a<b) {
	return 3;
    }
    return 1;
}

static int lt(int a,int b)
{
    if(a==b || b==-1) {
	return 2;
    }
    if(a<b) {
	return 1;
    }
    if(a>b) {
	return 3;
    }
    return 1;
}


static int cmp(int a[],int b[],int (*fun)(int,int),int index,int max)
{
    int c;
    if(index==max){
	return 0;
    }
    c=(*fun)(a[index],b[index]);
    switch(c)
    {
	case 1:
	    return 1;
	case 2:
	    return cmp(a,b,fun,index+1,max);
	case 3:
	    return 0;    
    }
    return 0;    
}
