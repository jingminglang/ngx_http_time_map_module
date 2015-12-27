# ngx_http_time_map_module

It is very easy to use this module set a variable dependent on localtime.
time_map only can use in the http block;

# Exmpale:
http {
...
    time_map $t1 {
     default 1; 
     gt  2015.*.*.15 10;
    }
    time_map $t2 {
     default 1; 
     lt  2015.*.*.15 10;
    }
    time_map $t3 {
     default 1; 
     at  2015.12.30.14  2015.12.30.15 10;
    }
    time_map $t3 {
     default 1; 
     at  2015.12.30.14  2015.12.30.15 1;
     noat  2015.12.29  2015.12.31 2;
    }
   
...
  server {
    if( $t1 = 10) {
     rewrite .....; #maybe some other actions.
    }
  }
}

default

gt

lt

at

noat

# TODO
