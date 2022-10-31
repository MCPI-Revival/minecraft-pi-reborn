pipeline {
    agent none
    stages {
        stage('Debian Buster') {
            agent {
                dockerfile {
                    filename 'scripts/ci/Dockerfile'
                    args '-v /var/run/docker.sock:/var/run/docker.sock --network host'
                }
            }
            stages {
                stage('Build') {
                    steps {
                        sh './scripts/ci/run.sh'
                    }
                    post {
                        success {
                            archiveArtifacts artifacts: 'out/*.AppImage*', fingerprint: true
                        }
                    }
                }
                stage('Publish') {
                    steps {
                        sh 'apt-get update && apt-get install -y docker.io'
                        sh 'rm -rf ./out/server-amd64'
                        sh './scripts/build.sh server amd64'
                        sh 'docker build --no-cache --tag thebrokenrail/minecraft-pi-reborn-legacy-server .'
                        withCredentials([usernamePassword(credentialsId: 'docker_hub_login', usernameVariable: 'DOCKER_HUB_USERNAME', passwordVariable: 'DOCKER_HUB_PASSWORD')]) {
                            sh 'docker login -u "${DOCKER_HUB_USERNAME}" -p "${DOCKER_HUB_PASSWORD}"'
                        }
                        sh 'docker push thebrokenrail/minecraft-pi-reborn-legacy-server'
                    }
                }
            }
        }
    }
}
