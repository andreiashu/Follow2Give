<?php

require_once(drupal_get_path('module', 'twitter_integration') . '/twitter_integration.common.inc');

function store_twitter_credentials_callback() {
  global $user;


  if(isset($_GET['oauth_token'])) {
    $account = twitter_return_load_user();
    twitter_user_save($account->twitter_token);
    db_query("INSERT IGNORE INTO {f2g_twitter_tokens} (uid, oauth_token, oauth_token_secret) VALUES (%d, '%s', '%s')",
      $account->uid, $account->twitter_token->oauth_token, $account->twitter_token->oauth_token_secret);

    $user = $account;
  }
  elseif(!$user->uid) {
    twitter_goto_twitter_login('store-twitter-credentials');
    //drupal_goto('twitter-login', array('destination' => 'store-twitter-credentials'));
  }

  $user_oauth = twitter_oauth_user_load($user->uid);
  if($user_oauth) {
    dpm($user_oauth, '$user_oauth');
    try {
    $twitterObj = new EpiTwitter(CONSUMER_KEY, CONSUMER_SECRET, $user_oauth->oauth_token, $user_oauth->oauth_token_secret);
    }
    catch(Exception $e) {
      dpm($e->getMessage(), 'eerror');
    }
    dpm($twitterObj, '$twitterObj');
    //$twitterInfo= $twitterObj->get_statusesFriends();
  }

  return 'store_twitter_credentials_callback';
}

function twitter_integration_login_callback() {
  twitter_integration_include();

  $twitterObj = new EpiTwitter(CONSUMER_KEY, CONSUMER_SECRET);

  try {
    $url = $twitterObj->getAuthorizeUrl(null,array('oauth_callback' => WEBSITE_URL . '/twitter-loggedin'));
  }
  catch(Exception $e) {
    dpm($e);

  }
  return '<a href="'.$url.'">Authorize with Twitter</a>';

  return 'test';
}



function twitter_integration_stored_data_test_callback($user) {
  $user_oauth = twitter_oauth_user_load($user->uid);
  try {
    $twitterObj = new EpiTwitter(CONSUMER_KEY, CONSUMER_SECRET, $user_oauth->oauth_token, $user_oauth->oauth_token_secret);
  }
  catch(Exception $e) {
    dpm($e->getMessage(), 'eerror');
  }
  dpm($twitterObj, '$twitterObj');

  return 'twitter_integration_stored_data_test_callback';
}