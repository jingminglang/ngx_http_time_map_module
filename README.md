# ngx_http_time_map_module

It is very easy to use this module set a variable dependent on localtime.

Use it like nginx map module and geo module.

*time_map* only can use in the http block;

notice: It's a new module maybe have some bugs but you can try it.

# Exmpale:

> http {  
> ...  
>     time_map $t1 {  
>      default 1;   
>      gt  2015.\*.\*.15 10;  
>     }  
>  
>     time_map $t2 {  
>      default 1;   
>      lt  2015.*.*.15 10;  
>     }  
>  
>     time_map $t3 {  
>      default 1;   
>      at  2015.12.30.14  2015.12.30.15 10;  
>     }  
>  
>     time_map $t3 {  
>      default 1;   
>      gt  2015.1.12.15.30 10;  
>      at  2015.12.30.14  2015.12.30.15 1;  
>      noat  2015.12.29  2015.12.31 2;  
>     }  
>      
> ...  
>   server {  
>     if( $t1 = 10) {  
>      rewrite .....; #maybe some other actions.  
>     }  
>   }  
> }  

# Directives

* time_map

> Syntax:	time_map  $variable { ... }  
> Default:	—  
> Context:	http  

* default

> Syntax:	default default_value
> Default:	—  
> Context:	time_map  

* gt

> Syntax:	gt Time value
> Default:	—  
> Context:	time_map  

If the request localtime is greater than the Time you set  then set Value to $variable.

* lt

> Syntax:	lt Time Value
> Default:	—  
> Context:	time_map  

If the request localtime is less than the Time you set then set Value to $variable.

* at

> Syntax:	at begin_time end_time value
> Default:	—  
> Context:	time_map  

* noat

> Syntax:	noat begin_time end_time value
> Default:	—  
> Context:	time_map  


# TODO

* use the cached time;
* include directive;


