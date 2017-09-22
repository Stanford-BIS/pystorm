do
```sh
docker-compose run quartus
```
which will drop you into a shell. Then

```sh
su <username>
```

Assuming you put the installation tarball into `./install` directory, do
```sh
cd /home/dev/install
tar -xf <tarball>
./setup.sh
```
It is assumed that you want to install Quartus to `/opt/quartus`. So make sure the directory exists on the host machine and that you have read/write permission. Inside the docker container, `/opt/quartus` is mounted to `/home/quartus`. So, during setup, specify `/home/quartus` as your installation directory.
