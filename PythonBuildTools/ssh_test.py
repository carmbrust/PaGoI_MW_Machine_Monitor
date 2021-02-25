import base64
import paramiko
key = paramiko.RSAKey(data=base64.b64decode(b'AAAAB3NzaC1yc2EAAAADAQABAAABAQC6bqYjY1h+0wbN3Xc2Y5k44QUDSpTOhCfuB1a0l2JCRwvKrlt9FpadVDHmK0fOAsmer0hgosTyI0NR7t+ulkK59Xd7+0PHPd15ZLRE5LdHxR5XdkqXaf4UM2R4IT41lDnBBM1siBZddt9TP0t1e6+2inIgv02VvUeS801W/VllTFE7Ud6vgHpbvxc1kUeqVXbJWo8A3VFnqnMcXNQUA6ta3O9623tNm2VkgniRnmvtdwJ7/DKUe+ov2sQ6uIyKUt85MtdjswwyUl3zhVQ4UzqJ6ozGtBIRTrvp5iTkzi802KhwzPI0ZsDJ4sKoPqHdLFeXJGLlU0fxNo3P24tkY9fz'))

# ssh-rsa AAAAB3NzaC1yc2EAAAABJQAAAQEAtGOw0BSe7lcA4jsgeotkNffiCdDfrYZFw0pdCKu6kPadk3buxKG/OUdCFWQS0MCwmYOF541l4rlQrOa/Win4tDnB00Nh0CHdRNg/yhyqYGpD3qd3jYHhgOrnFfN23ruDtDoQsX++HXf8OdP9CSzLwCAL8SeDkYQdFA7K3tmeHIW/6sKryWZD/gDxCz+3Qw7aKgXLpwgnIVqOq0zQ3Zyu4Kt2+dZMLmDd5M5ANYgdpSiLReUo+Pk5bq2oiTeyYOgIc9ErZ7PJ2G4d+O0t0BC/++6LAYZftOWKFd9np9B8IeI/MPJWfZiZs36JnidLBcA92A5Cicxy2eegEbojgn9Mpw== rsa-key-20210122



client = paramiko.SSHClient()
client.get_host_keys().add('rpi3-md.local', 'ssh-rsa', key)
client.connect('rpi3-md.local', username='pi', password='MILwright!')
stdin, stdout, stderr = client.exec_command('ls -lt')
for line in stdout:
    print('... ' + line.strip('\n'))
client.close()
