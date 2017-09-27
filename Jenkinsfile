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
                            sh "docker build --file docker/Dockerfile_compile_FPGA -t ${lowercase_tag}_fpga ."
                            sh "docker run --rm -v ${WORKSPACE}/artifacts:/artifacts ${lowercase_tag}_fpga"
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
