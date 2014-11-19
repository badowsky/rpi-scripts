#include <linux/module.h>
#include <linux/init.h>
#include <linux/kmod.h>


static int umh_test( void )
{
  struct subprocess_info *sub_info;
  char *argv[] = { "/home/pi/rpi-scripts/special_button_module/run_pressed.sh", NULL };
  static char *envp[] = {
        "HOME=/",
        "TERM=linux",
        "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };

  sub_info = call_usermodehelper_setup( argv[0], argv, envp, UMH_WAIT_PROC );
  if (sub_info == NULL) return -ENOMEM;

  return call_usermodehelper_exec( sub_info, UMH_WAIT_PROC );
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

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR("Mateusz Badowski");
MODULE_DESCRIPTION("LCD 2x16 Char Device");