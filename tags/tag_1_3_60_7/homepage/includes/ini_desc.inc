  <!-- CONTENT INI DESCRIPTION PAGE START -->
  <td style="vertical-align: top;" rowspan="1" colspan="1" align="left">
    <table cellpadding="10" cellspacing="0" width="100%" align="left">
      <tr>
        <td valign="top">
	<h1>General configuration of version 1.2<br>this does not apply to version 1.3!</h1>
	<table cellspacing="1" border="1">
	<tr>
	<td><b>NAME</b></td><td><b>DESCRIPTION</b></td>
	</tr>
	<tr>
	<td>import_putty_ssh_sessions</td><td><ul><li>0 - do not import putty's sessions</li><li>1 - import sessions and ask for passwords</li><li>2 - import sessions and don't ask for passwords (e.g, if you're using pageant instead)</li></ul></td>
	</tr>
	<tr>
	<td>import_sshcom_ssh_sessions</td><td>if not empty, this value is path to sshcom's sessions</td>
	</tr>
	<tr>
	<td>passwd_crypter</td><td>the path to PasswordCrypter's dll, which is used to store password encrypted instead of plaintext. leave this blank if you don't want to encrypt or store passwords.</td>
	</tr>
	<tr>
	<td>passwd_crypter_testpwd</td><td>this value is used to check your Masterpassword - for misstyping and so</td>
	</tr>
	<tr>
	<td>cache_fs</td><td>set to 1, if you wish to cache directory listings</td>
	</tr>
	</table>
	<h1>Connections configuration (Sections are counting from 1 upto the last one, e.g [1] or [2],...)</h1>
	<table cellspacing="1" border="1">
	<tr>
	<td><b>NAME</b></td><td><b>DESCRIPTION</b></td>
	</tr>
	<tr>
	<td>title</td><td>Name of connection, which shows in Total Commander's panel</td>
	</tr>
	<tr>
	<td>host</td><td>server to connect to</td>
	</tr>
	<tr>
	<td>username</td><td>connect as this user</td>
	</tr>
	<tr>
	<td>password</td><td>if using password authentification and this value is not empty, it's used as the password<br>if you're using a PasswordCrypter, this would be encrypted and you have to use the configuration dialog to change the value</td>
	</tr>
	<tr>
	<td>port</td><td>server listens on this port</td>
	</tr>
	<tr>
	<td>home_dir</td><td>after connecting to the server, change to this directory</td>
	</tr>
	<tr>
	<td>compression</td><td>if set(has value 1), data would be compressed</td>
	</tr>
	<tr>
	<td>use_key_auth</td><td>if set, use key authentification instead of password</td>
	</tr>
	<tr>
	<td>keyfilename</td><td>file which containes key for key-auth (you can leave this empty and use pageant instead)</td>
	</tr>
	<tr>
	<td>dont_ask4_passphrase</td><td>if set, the plugin will not ask you to enter passphrase for the key (while using key-auth)</td>
	</tr>
	<tr>
	<td>dont_ask4_username</td><td>if set, the plugin won't ask for username</td>
	</tr>
	<tr>
	<td>dont_ask4_password</td><td>if set, the plugin won't ask for password</td>
	</tr>
	<tr>
	<td>proxy_type</td><td>proxy type as defined in putty<br><ul><li>0 - none</li><li>1 - socks4</li><li>2 - socks5</li><li>3 - http</li><li>4 - telnet</li><li>5 - command</li></ul></td>
	</tr>
	<tr>
	<td>proxy_host</td><td>proxy to connect to</td>
	</tr>
	<tr>
	<td>proxy_port</td><td>proxy server's port</td>
	</tr>
	<tr>
	<td>proxy_username</td><td>username for proxy, if needed</td>
	</tr>
	<tr>
	<td>proxy_password</td><td>password for proxy, if needed</td>
	</tr>
	<tr>
	<td>proxy_telnet_command</td><td>proxy command, for specific proxy type</td>
	</tr>
	<tr>
	<td>set_mtime_after_put</td><td>if set, modified time for a file would be set (after put)</td>
	</tr>
	<tr>
	<td>chmod_value_put</td><td>chmod value would be set to this value (for file)</td>
	</tr>
	<tr>
	<td>chmod_value_mkdir</td><td>chmod value would be set to this value (for directories)</td>
	</tr>
	<tr>
	<td>set_chmod_after_put</td><td>if set, chmod would be called after put</td>
	</tr>
	<tr>
	<td>set_chmod_after_mkdir</td><td>if set, chmod would be called after make directory</td>
	</tr>
        </table>
        </td>
      </tr>
    </table>
  </td>
  <!-- CONTENT INI DESCRIPTION END -->
