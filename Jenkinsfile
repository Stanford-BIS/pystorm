pipeline {
    agent any

    stages {
        stage('Build') {
            steps {
                parallel(
                    "Software" : {
                        script {
                            def lowercase_tag = "${BUILD_TAG.toLowerCase()}"
                            sh "docker build --file docker/Dockerfile_compile_source -t ${lowercase_tag} ."
                        }
                    },
                    "FPGA" : {
                        script {
                            def lowercase_tag = "${BUILD_TAG.toLowerCase()}"
                            sh "docker build --file docker/Dockerfile_compile_FPGA -t quartus_fpga_build ."
                            sh "docker/build_fpga.sh ${WORKSPACE} quartus_fpga_build"
                        }
                    }
                )
            }
        }
        stage('Test') {
            steps {
                script {
                    def lowercase_tag = "${BUILD_TAG.toLowerCase()}"
                    sh "docker/run_docker.sh ${lowercase_tag}"
                }
            }
        }
    }
}
