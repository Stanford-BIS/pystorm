def HOSTIP = 'NONE'
def JENKINS_HOST_PATH = 'NONE'

pipeline {
    agent any

    stages {
        stage('Initialize') {
            steps {
                script {
                    HOSTIP = "${sh returnStdout: true, script: 'ip route show | awk \'/default/ {print \$3}\''}".trim()
                    JENKINS_HOST_PATH = "${sh returnStdout: true, script: 'echo ${WORKSPACE} | sed -e \'s/^\\/var/\\/home\\/jenkins/g\''}".trim()
                }
                sh script: "git clean -xfd"
                sh script: "mkdir -p artifacts"
                sh script: "mkdir -p build/release"
                sh script: "mkdir -p build/build"
                sh script: "ssh ${HOSTIP} 'docker build -t stanfordbis/ubuntu-pystorm:latest --file=${JENKINS_HOST_PATH}/docker/Dockerfile_build_environment_image ${JENKINS_HOST_PATH}/docker'"
                sh script: "ssh ${HOSTIP} 'docker build -t centos_quartus:latest --file=${JENKINS_HOST_PATH}/docker/Dockerfile_quartus_environment_image ${JENKINS_HOST_PATH}/docker'"
                sh script: "ssh ${HOSTIP} 'docker build -t pystorm_build --file=${JENKINS_HOST_PATH}/docker/Dockerfile_compile_source ${JENKINS_HOST_PATH}/docker'"
                sh script: "ssh ${HOSTIP} 'docker build -t quartus_fpga_build --file=${JENKINS_HOST_PATH}/docker/Dockerfile_compile_FPGA ${JENKINS_HOST_PATH}/docker'"
            }
        }
        stage('Build & Test'){
            parallel {
                stage('FPGA'){
                    steps {
                            sh script: "ssh ${HOSTIP} 'docker run --rm -i --user=\$(id -u jenkins) -v /home/quartus:/home/quartus -v ${JENKINS_HOST_PATH}:/pystorm quartus_fpga_build'"
                            archiveArtifacts 'artifacts/OKCoreBD.rbf,artifacts/setup.rpt,artifacts/hold.rpt,artifacts/recovery.rpt,artifacts/removal.rpt'
                    }
                }
                stage('Release'){
                    steps {
                        sh script: "ssh ${HOSTIP} 'docker run --rm -i --user=\$(id -u jenkins) -v ${JENKINS_HOST_PATH}:/pystorm pystorm_build /bin/bash -c \'cd build/release && cmake -G Ninja -DBD_COMM_TYPE=MODEL -DCMAKE_BUILD_TYPE=Release .. && cmake --build . --config Release && ctest -C Release -T test -VV --timeout 300\''"
                    }
                }
                stage('Debug'){
                    steps {
                        sh script: "ssh ${HOSTIP} 'docker run --rm -i --user=\$(id -u jenkins) -v ${JENKINS_HOST_PATH}:/pystorm pystorm_build /bin/bash -c \'cd build/debug && cmake -G Ninja -DBD_COMM_TYPE=MODEL -DCMAKE_BUILD_TYPE=Debug .. && cmake --build . --config Release && ctest -C Release -T test -VV --timeout 300\''"
                    }
                }
            }
        }
    }
}
