  <!-- CONTENT FRONT PAGE START -->
  <td style="vertical-align: top;" rowspan="1" colspan="1" align="left">
    <table cellpadding="10" cellspacing="0" width="100%" align="left">
      <tr>
        <td valign="top">
          <h1>What is FTPS?</h1>
          <p>SFTP is a subsystem of SSH, it's a secure way to copy files</p>

          <h1>What is SFTP4TC?</h1>
          <p>
            A file system plugin for <a href="http://ghisler.com/">Total Commander</a> which allows
            SFTP sessions. This is similar to the <a href="http://sourceforge.net/projects/ftps4tc/">
            FTPS Plugin</a> which enhances FTP with SSL/TLS.
          </p>

          <h1>Why (another) plugin for FTP?</h1>
          <p>
          Swiss export restrictions prohibit extending <a href="http://ghisler.com/">Total Commander's</a> FTP support
          to include Secure Connections. This plugin allows you to combine the power of Total Commander
          with the security of <a href="http://www.chiark.greenend.org.uk/~sgtatham/putty/download.html" target="_blank">PuTTY</a>'s implementation of SSH.

          SFTP requires SSH with the SFTP subsystem.
          </p>
          
          <h1>Project Status</h1>
          <p>
		  The plugin was moved from it's <a href="http://www.p42.net/total_commander/">original page</a> to this site on 26.9.2004.
		  It was developed by Hans-Juergen Petrich and is now maintained by Martin Kanich and <a href="http://www.franke.ms" target="_blank">Stefan "Bebbo" Franke</a>.
		  It should be the start for giving you better chance to request features and submits bugs or even join the development.
		  Now as you have public access to source repository, there's no need to publish source snapshots, but if you wish that, just send email or use the forums.
	      </p>
	      <p>
	      It's not trivial to add SCP support, but it could be the next feature or another plugin, so if you like to join, just send an email.
	      </p>
	      <p>
	      Next changes on the ToDo-list are:
	      </p>
	      <ul>
	       <li>fix remaining bugz</li>
        </ul>

        <p>
        New features in version 1.3 are:
        </p>
        <ul>
          <li>Usage of the <a href="http://www.chiark.greenend.org.uk/~sgtatham/putty/download.html" target="_blank">PuTTY</a> config dialog and thus removal of own configuration. <br/>
          The ini file and the password crypter are not supported in 1.3:
          <ul>
            <li>The configuration <b>is</b> the <a href="http://www.chiark.greenend.org.uk/~sgtatham/putty/download.html" target="_blank">PuTTY</a> configuration, new connections are also available in <a href="http://www.chiark.greenend.org.uk/~sgtatham/putty/download.html" target="_blank">PuTTY</a>.</li>  
            <li>Use a key pair and <a href="http://www.chiark.greenend.org.uk/~sgtatham/putty/download.html" target="_blank">Pageant</a> instead of passwords.</li>
          </ul>
          <li>added support for automatic transfer mode</li>          
        </ul>
	      <p>
	      New features in version 1.2 are:
	      </p>
	      <ul>
	      <li>encrypting passwords (but you should use keys every time you can)</li>
	      <li>configuration dialog (you can still edit your inis manually)</li>
	      <li>better installer (hope that's true:-)</li>
              </ul>

          <h1>How can I use SFTP with Total Commander after installing the plugin?</h1>
          <p>
          After installing an File-System plugin, you have to select "Network neighborhood". Here you can find "Secure FTP Connections". 
          </p>

          <h1>Encrypting passwords</h1>
          <p>
	  If you installed Password-Crypter you have to enter master password, which will be used to encrypt all passwords for your connections.
	  If you use keys or just don't save password, or you just don't care about plain password - just don't install password crypter or remove the line
	  beginning with "passwd_crypter=" in the ini.
          </p>

          <hr noshade="noshade" size="1" />
        </td>
      </tr>
    </table>
  </td>
  <!-- CONTENT FRONT PAGE END -->
