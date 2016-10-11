Vagrant.configure(2) do |config|
  config.vm.box = "ubuntu/trusty64"
  config.vm.define "connect", primary: true

  config.vm.define "connect" do |connect|
    connect.vm.hostname = "connect"
    connect.vm.network "private_network", type: "dhcp"
    connect.vm.synced_folder ".", "/home/vagrant/connect", type: "nfs"
    connect.vm.provision "shell", path: "bootstrap.sh", privileged: false
    connect.vm.provider "virtualbox" do |vb|
      vb.memory = 4096
      vb.cpus = 4
      vb.customize [ "guestproperty", "set", :id, "/VirtualBox/GuestAdd/VBoxService/--timesync-set-threshold", 1000 ]
    end
  end
end
