#!/bin/sh

dig @resolver4.opendns.com myip.opendns.com +short -4 > IPV4
scp IPV4 oncoto@oncoto.app:/srv/hermes/IPV4
