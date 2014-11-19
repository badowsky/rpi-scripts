#include <linux/module.h>
#include <linux/init.h>
#include <linux/kmod.h>

MODULE_LICENSE( "GPL" );


static int umh_test( void )
{
  struct subprocess_info *sub_info;
  char *argv[] = { "/usr/bin/logger", "help!", NULL };
  static char *envp[] = {
        "HOME=/",
        "TERM=linux",
        "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };

  sub_info = call_usermodehelper_setup( argv[0], argv, envp, GFP_ATOMIC );
  if (sub_info == NULL) return -ENOMEM;

  return call_usermodehelper_exec( sub_info, UMH_WAIT_PROC );
}

static int umh_test2( void )
{
  char *argv[] = { "/usr/bin/logger", "help!", NULL };
  static char *envp[] = {
        "HOME=/",
        "TERM=linux",
        "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };

  return call_usermodehelper( argv[0], argv, envp, UMH_WAIT_PROC );
}




static int __init mod_entry_func( void )
{
  return umh_test();
}


static void __exit mod_exit_func( void )
{
  return;
}

module_init( mod_entry_func );
module_exit( mod_exit_func );