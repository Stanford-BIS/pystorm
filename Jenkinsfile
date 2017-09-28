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
                    sh script: "git clean -xfd"
                    sh script: "mkdir -p artifacts"
                    sh script: "mkdir -p build"
                }
            }
        }
        stage('Build') {
            steps {
                parallel(
                    "Software" : {
                        script {
                            sh script: "ssh ${HOSTIP} 'docker build -t pystorm_build --file=${JENKINS_HOST_PATH}/docker/Dockerfile_compile_source ${JENKINS_HOST_PATH}/docker'"
                            sh script: "ssh ${HOSTIP} 'docker run --rm -i --user=\$(id -u jenkins) -v ${JENKINS_HOST_PATH}:/pystorm pystorm_build'"
                        }
                    },
                    "FPGA" : {
                        script {
                            sh script: "ssh ${HOSTIP} 'docker build -t quartus_fpga_build --file=${JENKINS_HOST_PATH}/docker/Dockerfile_compile_FPGA ${JENKINS_HOST_PATH}/docker'"
                            sh script: "ssh ${HOSTIP} 'docker run --rm -i --user=\$(id -u jenkins) -v /home/quartus:/home/quartus -v ${JENKINS_HOST_PATH}:/pystorm quartus_fpga_build'"
                            archiveArtifacts 'artifacts/OKCoreBD.rbf,artifacts/setup.rpt,artifacts/hold.rpt,artifacts/recovery.rpt,artifacts/removal.rpt'
                        }
                    }
                )
            }
        }
        stage('Test') {
            steps {
                script {
                    sh script: "ssh ${HOSTIP} 'docker run --rm -i --user=\$(id -u jenkins) -v ${JENKINS_HOST_PATH}:/pystorm pystorm_build ctest -C Debug -T test -VV --timeout 300'"
                }
            }
        }
    }
}